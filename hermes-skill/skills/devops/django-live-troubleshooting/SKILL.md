---
name: django-live-troubleshooting
title: Django Live Troubleshooting
description: Use when debugging live Django/Gunicorn servers.
---
# Django Production Zero-Downtime Troubleshooting and Verification

Procedural workflow for hot-patching and debugging live Django application servers running with Nginx + Gunicorn without introducing 500 errors.

## Trigger Conditions
- Hot-patching code on a live VPS.
- Restarting or reloading Gunicorn/ASGI servers.
- Syncing live state or troubleshooting 500/502 errors.

## Recommended Workflow

### 1. Verification Before Restarts
Always check Django's internal system check and URLs configuration before reloading Gunicorn or Nginx:
```bash
python manage.py check
```
If this fails, do NOT restart Gunicorn, as it will crash instantly and start throwing 500 errors to visitors.

### 2. Zero-Downtime Hot-Reloading
Instead of forcefully killing Gunicorn with `pkill -9 gunicorn`, perform a graceful reload (HUP signal) which allows existing workers to finish their requests while loading the new code configuration:
```bash
kill -HUP <gunicorn_master_pid>
```
To find the master Gunicorn process:
```bash
ps aux | grep gunicorn
```

### 3. Immediate Live Verification
Always perform localized cURL requests on the loopback interface (`127.0.0.1`) for main pages immediately after a reload to guarantee the site works before declaring completion:
```bash
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8000/
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8000/cart/
```

### 4. Gunicorn Log Auditing
If cURL returns a 500, immediately tail Gunicorn's actual stdout/stderr file to find the Python stack trace:
```bash
tail -n 100 /tmp/gunicorn.log
```

## Pitfalls to Avoid
- **Python Docstring Syntax Crashes:** Python docstrings (`"""..."""`) converted or appended via inline sed/cat or multi-line shell commands must have their quotation marks carefully escaped or completely stripped. A docstring like `"""Exposes database""" to templates` produces raw syntax errors in Python and crashes Gunicorn with a 500 error. Always run `python manage.py check` before reloading.
- **Session Expiry Pitfalls:** Setting `SESSION_EXPIRE_AT_BROWSER_CLOSE = True` wipes visitor carts when they close tabs. Use explicit `SESSION_COOKIE_AGE` instead to ensure state persistence.
- **Context Processor Missing Keys:** When updating templates, ensure context variables (e.g. `cart_items` vs `items`) match exactly what's sent by view methods or context processors.
- **Incorrect String Conversions in sed:** When executing automated search/replace operations, escape Python docstrings properly. A bare docstring like `"""Exposes live edit content database"""` with text appended to it can lead to fatal syntax errors. Always test with `python manage.py check` before restarting.
- **Dynamic Content Injection Keys / Missing Attributes:** When saving dynamic frontend configurations to dytamic-friendly tables (e.g. `SiteSetting` / CMS Configuration), make sure the fields you are saving actually exist in the model. Saving to a non-existent field (like a fallback `.description` when it is actually `.about_text`) will fail silently at the Django ORM layer during AJAX POSTs and changes will disappear after page refresh. Verify model schemas by inspecting fields via Python reflection before committing to a field name.
- **Nested {% if %} Blocks in base.html:** Automatic insertion/injection of JS or custom blocks into template footers must not duplicate block-level tags like `{% if user.is_staff %}` as they will orphan existing `{% endif %}` structures and throw instant 500 template syntax errors.

