---
name: django-deployment
description: "Deploy Django projects to production: Gunicorn, Whitenoise, low-RAM setups, Iranian CDN workarounds"
version: 1.0.0
author: Hermes Agent
license: MIT
platforms: [linux]
metadata:
  hermes:
    tags: [django, deployment, gunicorn, whitenoise, production, iran, cdn-bypass]
    related_skills: [systematic-debugging, plan]
---

This is a skill the user may invoke when debugging deployment problems. When this user (Mahbod, Persian speaker, ultra-terse preference) asks deployment questions:

- **Communicate in Persian** by default (casual/friendly, NOT formal)
- **Ultra-terse format**: abbreviate, use arrows (→), no filler sentences
- **No markdown tables** — compact inline lists only (user reads on mobile Telegram)
- After delivering a fix: confirm with `✅ fix: [what changed].` and **stop** — do NOT re-explain or continue unless asked
- **Do NOT re-explain what the fix does** after they've seen it work

---

# Django Production Deployment

## When to use
- User asks to deploy a Django project to a production server
- Setting up production-ready Django with admin panel (Unfold/Jazzmin)
- Django needs to work on low-RAM VPS (<1GB) or for Iran-based users (CDN restrictions)

## Communication Style (MANDATORY — see 'persian-speaking-technical-peer' skill)
This skill carries the user's communication preferences in its own SKILL.md.
Load it for full details (ultra-terse, Persian, peer-critic, project separation, etc.).
Key rules applicable here:
- **Persian by default** (casual/friendly, NOT formal)
- **Ultra-terse format**: abbreviate, use → arrows, no filler sentences
- **After fix**: confirm with `✅ fix: [what changed].` and **stop** — do NOT re-explain
- **Never re-explain what the fix does** after they've seen it work

## Steps

### 1. Environment Setup
```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

Minimum reqs.txt for a typical project:
```
django>=5.0
gunicorn
whitenoise
psycopg2-binary
django-unfold         # if using Unfold admin
python-dotenv         # for .env loading
python-telegram-bot   # if bot attached
celery[redis]         # if background tasks
redis                 # if background tasks
Pillow                # if ImageField used
```

### 2. Settings Production Hardening

**Secrets → `.env`:**
```python
import os
from pathlib import Path
from dotenv import load_dotenv

BASE_DIR = Path(__file__).resolve().parent.parent
load_dotenv(BASE_DIR / ".env")

SECRET_KEY = os.environ.get("SECRET_KEY")
DEBUG = os.environ.get("DEBUG", "False") == "True"
ALLOWED_HOSTS = os.environ.get("ALLOWED_HOSTS", "*").split(",")
```

**Static files with Whitenoise** (Django won't serve static in production):
```python
MIDDLEWARE = [
    'django.middleware.security.SecurityMiddleware',
    'whitenoise.middleware.WhiteNoiseMiddleware',  # ← Must be right after Security
    ...
]

STATIC_URL = 'static/'
STATIC_ROOT = BASE_DIR / 'staticfiles'
STATICFILES_STORAGE = 'whitenoise.storage.CompressedManifestStaticFilesStorage'
```

Then run: `python manage.py collectstatic --noinput`

**Unfold admin — Iran CDN fix:**
Google Fonts + Material Symbols CDNs are blocked in Iran. The page loads but hangs on font requests. Fix:
```python
UNFOLD = {
    "COLORS": {},        # ← needed to suppress CDN color defaults
    "FONTS": [],         # ← disables Google Fonts CDN fetch
    ...
}
```
After changing UNFOLD config, re-run `collectstatic --clear`.

### 3. Gunicorn — Match Workers to RAM

| RAM  | Workers | Command |
|------|---------|---------|
| 512MB | 1 | `--workers 1` |
| 1GB   | 2 | `--workers 2` |
| 4GB+  | `2*$(nproc)+1` | `--workers $((2 * $(nproc) + 1))` |

```bash
gunicorn website.wsgi:application \
  --bind 0.0.0.0:8000 \
  --workers 1 \
  ### 4. Template Gotcha — No Method Chaining

  Django template engine does NOT allow `.filter(...).first()`.

  **Bad:** `{% with img=p.images.filter(is_primary=True).first %}`  
  `TemplateSyntaxError: Could not parse the remainder: '(is_primary).first'`

  **Bad (model class, no .objects):** `Product.select_related("type")`  
  `AttributeError: Manager isn't available; 'Product' is abstract or is not a model.`  
  This happens when you call `.select_related()` directly on the model class instead of `Product.objects.select_related(...)`.

  **Good — Pre-compute in the view:**
  ```python
  def product_list(request):
      # Always use .objects!
      qs = Product.objects.select_related("type").order_by("-id")
      for p in qs:
          # Pre-compute querysets that you need in templates
          p.primary_image = p.images.filter(is_primary=True).first()
      return render(request, "list.html", {"products": qs})
  ```

### 4.5 Nginx Static File Permissions — 403 Fix

Nginx runs as user `www-data`. If static files live under `/root/something/`, Nginx gets **403 Forbidden**.

**Symptom:** Page loads without CSS. `GET /static/unfold/css/styles.css → 403` in browser dev tools.

**Fix — move to world-readable location:**

```bash
cp -r /path/to/project/staticfiles /var/www/project-static
chown -R www-data:www-data /var/www/project-static
```

Then update Nginx config `location /static/` block to point to the new path, and reload: `nginx -t && systemctl reload nginx`.

### 4.7 Systemd Service Patterns for Django + Friends

On Ubuntu/Debian systems, use systemd for reliability (restart on crash, start on boot).

**Rule: WorkingDirectory must point to the dir where `wsgi.py` lives, not the project root.**

If `wsgi.py` lives at `/root/project/website/website/wsgi.py`, set:
```
WorkingDirectory=/root/project/website/website
```
Not `/root/project/website/`. Gunicorn imports `website.wsgi:application` from Python's path, and Python needs `website/` to be a detectable package under the working dir.
```ini
[Unit]
Description=Gunicorn daemon for Django
After=network.target redis-server.service

[Service]
User=root
WorkingDirectory=/path/to/project/website  # ← dir containing wsgi.py as sub-package
ExecStart=/path/to/venv/bin/python3 -m gunicorn website.wsgi:application --bind 127.0.0.1:8000 --workers 2
Restart=always

[Install]
WantedBy=multi-user.target
```

**Celery worker service:**
```ini
[Unit]
Description=Celery Worker
After=network.target redis-server.service

[Service]
User=root
WorkingDirectory=/path/to/project/website
ExecStart=/path/to/venv/bin/celery -A website worker -l info --concurrency=2
Restart=always

[Install]
WantedBy=multi-user.target
```

Key: Ensure `celery.py` exists in the Django app dir and `__init__.py` imports the Celery app. The `-A website` refers to a module where the django settings are accessible.

**Flask auxiliary server (e.g. parking dashboard) service:**
```ini
[Unit]
Description=Flask Auxiliary Server
After=network.target

[Service]
User=root
WorkingDirectory=/path/to/project/server/dir
ExecStart=/path/to/venv/bin/python3 server.py
Restart=always

[Install]
WantedBy=multi-user.target
```
Flask reads files like `/tmp/dashboard.html` at process level — if the server.py references external files (e.g. `with open('/tmp/dashboard.html')`), those files must EXIST on the destination server before starting the service. Missing file → crash on boot.

**Node app (e.g. 9router) service:**
```ini
[Unit]
Description=Node Service
After=network.target

[Service]
User=root
ExecStart=/usr/bin/npx --yes app-name --flags --port 20128
Restart=always

[Install]
WantedBy=multi-user.target
```
Use `npx --yes` instead of manual `node /path/to/cli.js` to avoid permission issues when the binary is owned by another user (e.g. `ubuntu:ubuntu`). `npm install -g` times out → skip it, npx handles it on demand.

**Service lifecycle commands:**
```bash
systemctl daemon-reload                    # after creating/changing any .service file
systemctl enable service-name              # start on boot
systemctl start service-name               # start now
systemctl restart service-name             # stop + start
systemctl status service-name --no-pager   # check status (no less piped)
systemctl is-active service-name           # returns "active" or "failed"
journalctl -u service-name --no-pager -n 30  # check last 30 log lines
```

### 4.8 Nginx bind() to Port Already in Use — Recovery

When Nginx config has a `listen` block for a port already held by another process (e.g. Flask on 5000), Nginx refuses to start at all — even its other listeners (80, 443) go down.

**Symptom:** `systemctl restart nginx` fails with `bind() to 0.0.0.0:5000 failed (98: Address already in use)`. All web services go down. The main site on port 80/443 is unreachable.

**Fix — separate the offending listen block into its own site file, or remove it:**

**Strategy A (recommended):** Remove the `listen` block from Nginx entirely. Access the auxiliary service via a sub-path (`/parking/` → proxy_pass http://127.0.0.1:5000) instead of a separate port.

**Strategy B (port must stay):** Create a separate nginx site file for the auxiliary port:
```nginx
server {
    listen 5000 ssl http2;
    server_name domain.com;
    ssl_certificate ...;
    ssl_certificate_key ...;
    location / { proxy_pass http://127.0.0.1:5000; }
}
```
Keep it in a separate `sites-available/aux` file so a bind failure there doesn't affect the main site.

**Never put auxiliary server `listen` blocks in the same nginx config as the main site — a bind failure on the auxiliary port takes down the entire site.**

### 4.9 SSL Certificate (Let's Encrypt) for New Domain

To put the site on a domain instead of `IP:8000`:

```bash
apt-get install -y nginx
```

Create `/etc/nginx/sites-available/resin`:

```nginx
server {
    listen 80;
    server_name your-domain.com;

    location / {
        proxy_pass http://127.0.0.1:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    }

    location /static/ {
        alias /var/www/project-static/;
    }

    location /media/ {
        alias /path/to/your/media/;
    }
}
```

Enable:
```bash
ln -sf /etc/nginx/sites-available/resin /etc/nginx/sites-enabled/
rm -f /etc/nginx/sites-enabled/default
nginx -t && systemctl restart nginx
```

Now accessible at `http://your-domain.com` (port 80).

## 4.6 Telegram Bot Variable Name — Unify Across Files

The bot file (`store/bot.py`) often has a variable named `Token` that references `settings.TELEGRAM_BOT_TOKEN`. This causes NameError if the variable name drifts. Fix by defining all Telegram env vars at the top of the bot file:

```python
TELEGRAM_BOT_TOKEN = getattr(settings, "TELEGRAM_BOT_TOKEN", "") or ""
TELEGRAM_CHANNEL_CHAT_ID = getattr(settings, "TELEGRAM_CHANNEL_CHAT_ID", "") or ""
TELEGRAM_LOG_CHAT_ID = getattr(settings, "TELEGRAM_LOG_CHAT_ID", "") or ""
```

Then use `TELEGRAM_BOT_TOKEN` everywhere, not `Token`.

### 4.6.1 Critical: Always Use Server's Telegram Bot Token

**Problem**: Local `.env` often has stale token. Production token lives in Django settings on server (`/root/resin-web/website/.env`).

**Symptom**: `Unauthorized` / `InvalidToken` when sending from local machine.

**Fix**: Always fetch current token from server before running local scripts, or run cron jobs on server where `.env` is auto-loaded.

```bash
# Get current token from server
sshpass -p 'password' ssh root@server_ip "grep TELEGRAM_BOT_TOKEN /root/resin-web/website/.env | cut -d'=' -f2"

# Or run cron jobs on server where .env is auto-loaded
0 7 * * * cd /root && source /root/resin-web/website/.env && python3 /root/.hermes/scripts/send_tech_posts.py
```

**Reference**: See `cron-notifications/references/telegram-bot-token-management.md` for complete token management guide.

### 4.7 Python-Telegram-Bot Version & Dependencies — Critical for Deployment

**Critical:** `python-telegram-bot>=22.0` requires specific dependency versions:

- **`httpx >= 0.27.0`** (for `HTTPXRequest.httpx_kwargs` parameter support)
- **`httpcore >= 1.0.0`** (httpx dependency)
- **`anyio >= 4.0`** (httpx dependency)

**Ubuntu/Debian apt packages are TOO OLD:**
- `python3-httpx` = 0.26.0 (too old, missing `httpx_kwargs`)
- `python3-httpcore` = 1.0.2 (OK but paired with old httpx)
- `python3-python-telegram-bot` = 20.8 (too old, missing `httpx_kwargs`)

**Fix — Install via pip with --break-system-packages:**
```bash
pip3 install python-telegram-bot==22.5 httpx==0.28.1 httpcore==1.0.9 anyio==4.8.0 --break-system-packages --quiet
```

**Verify on deployment:**
```bash
python3 -c "import telegram; import httpx; import httpcore; print(f'telegram:{telegram.__version__} httpx:{httpx.__version__} httpcore:{httpcore.__version__}')"
# Should output: telegram:22.x httpx:0.28.x httpcore:1.0.x
```

**If using a virtualenv (e.g. Hermes gateway), install there:**
```bash
/path/to/venv/bin/pip install python-telegram-bot==22.5 httpx==0.28.1 httpcore==1.0.9 --quiet
```

### 4.7 Python-Telegram-Bot Version & Dependencies — Critical for Deployment

**Critical:** `python-telegram-bot>=22.0` requires specific dependency versions:

- **`httpx >= 0.27.0`** (for `HTTPXRequest.httpx_kwargs` parameter support)
- **`httpcore >= 1.0.0`** (httpx dependency)
- **`anyio >= 4.0`** (httpx dependency)

**Ubuntu/Debian apt packages are TOO OLD:**
- `python3-httpx` = 0.26.0 (too old, missing `httpx_kwargs`)
- `python3-httpcore` = 1.0.2 (OK but paired with old httpx)
- `python3-python-telegram-bot` = 20.8 (too old, missing `httpx_kwargs`)

**Fix — Install via pip with --break-system-packages:**
```bash
pip3 install python-telegram-bot==22.5 httpx==0.28.1 httpcore==1.0.9 anyio==4.8.0 --break-system-packages --quiet
```

**Verify on deployment:**
```bash
python3 -c "import telegram; import httpx; import httpcore; print(f'telegram:{telegram.__version__} httpx:{httpx.__version__} httpcore:{httpcore.__version__}')"
# Should output: telegram:22.x httpx:0.28.x httpcore:1.0.x
```

**If using a virtualenv (e.g. Hermes gateway), install there:**
```bash
/path/to/venv/bin/pip install python-telegram-bot==22.5 httpx==0.28.1 httpcore==1.0.9 --quiet
```

### 5. Login Loop Fix
After login, user is redirected back to login page (302 → login again, never reaching dashboard).

**Root cause most of the time:** `DEBUG=False` causes Django to set `CSRF_COOKIE_SECURE` and `SESSION_COOKIE_SECURE` to **True** by default (even if not explicitly set). On plain HTTP (no HTTPS), the browser refuses to attach these secure cookies, so every POST (login form submission) fails CSRF validation silently — the form re-renders without error.

**Fix — explicit secure=False for HTTP:**
```python
CSRF_COOKIE_SECURE = False      # allow CSRF cookie over HTTP
SESSION_COOKIE_SECURE = False   # allow session cookie over HTTP
SESSION_COOKIE_HTTPONLY = True  # prevent JS access (extra hardening)
SESSION_EXPIRE_AT_BROWSER_CLOSE = True  # auto-logout on tab close
```

**Other checks:**
- **Cache**: Clear browser cache + cookies for the domain, or test in **Incognito/Private** window
- **Database sessions**: Run `python manage.py clearsessions` to purge stale session rows
- **Verify superuser**: `echo "from django.contrib.auth import get_user_model; User = get_user_model(); u = User.objects.filter(is_superuser=True).first(); print(f'{u.username} superuser={u.is_superuser}')" | python manage.py shell`
- **Check gunicorn access log**: A `302` on `POST /admin/login/` means login failed. A `200` with login page HTML (12807 bytes typical) means same — form re-rendered. A `302` to `/admin/` means success.

### 5.5 Product Auto-Code Generation — Filter by Type

When a Product model generates codes automatically via `save()`, the naive approach uses a number-range filter that accidentally includes products from other types:

**Bad — range filter:**
```python
max_code = Product.objects.filter(
    code__gte=start, code__lt=start + 10000
).aggregate(m=Max("code"))["m"]
```
If type A starts at 7000 and type B starts at 8000, both fall in the 7000–17000 range, so `max_code` returns 8000 (from type B), causing the new product of type A to get code 8001 instead of 7001.

**Fix — filter by type:**
```python
max_code = Product.objects.filter(type=self.type).aggregate(m=Max("code"))["m"]
self.code = start if max_code is None else max_code + 1
```
This ensures each product type has an independent code sequence.

### 5.6 MEDIA_ROOT Must Match Nginx Alias

Django writes uploads to `MEDIA_ROOT`. Nginx reads from its `location /media/ alias`. If these paths differ, uploads work but images 404 on the storefront.

```python
# settings.py
MEDIA_ROOT = "/var/www/my-media"   # ← must match Nginx alias
```
```nginx
location /media/ { alias /var/www/my-media/; }
```
Then: `chown -R www-data:www-data /var/www/my-media`

### 5.7 Running Celery Tasks Without Redis (Low-RAM Alternative)

If RAM is <1GB, Redis + Celery Worker + Celery Beat can trigger OOM. Replace with a cron job that runs the task directly in the Django context.

Create `/opt/run-promotion.sh`:

```bash
#!/bin/bash
cd /path/to/project/website
source ../venv/bin/activate
python -c "
import django, os
os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'website.settings')
django.setup()
from store.tasks import process_promotion_queue
process_promotion_queue()
"
```

Then add to user crontab:
```bash
chmod +x /opt/run-promotion.sh
crontab -e
# 0 10 * * * /opt/run-promotion.sh >> /var/log/promotion-cron.log 2>&1
```

This runs the daily promotion post without needing any Celery or Redis processes.

### 5.8 Celery 5.x Compatibility — No `--daemon` Flag

**⚠️ CRITICAL:** Celery 5.x removed the `--daemon` flag. Any deployment script using:
```bash
celery -A website beat -l WARNING --daemon --pidfile /tmp/celery-beat.pid
celery -A website worker -l INFO --concurrency 2 --daemon --pidfile /tmp/celery-worker.pid
```
**This silently fails** — celery starts then exits immediately. Worker/beat never run.

**Fix — use `nohup` with explicit backgrounding:**
```bash
nohup celery -A website worker -l INFO --concurrency 2 \
  > /tmp/celery-worker.log 2>&1 &
echo $! > /tmp/celery-worker.pid

nohup celery -A website beat -l WARNING \
  > /tmp/celery-beat.log 2>&1 &
echo $! > /tmp/celery-beat.pid
```
Also update any `run.sh` — remove `--daemon` flags everywhere.

### 6. Firewall
```bash
ufw allow 8000/tcp
```
On cloud panels (Arvan, Hetzner, DigitalOcean), also open the port in the web dashboard.

### 7. Starting Services on Low-RAM
Don't run everything at once — start gunicorn alone first, add others if memory permits:
```bash
# Step 1: Kill leftovers
fuser -k 8000/tcp 2>/dev/null

# Step 2: Start gunicorn
gunicorn website.wsgi:application \
  --bind 0.0.0.0:8000 \
  --workers 1 \
  --timeout 120 \
  --access-logfile /tmp/gunicorn.log &

# Step 3 (if > 768MB free): Redis + Celery
redis-server --daemonize yes
celery -A website worker -l INFO --daemon
celery -A website beat -l WARNING --daemon

# Step 4 (always): Telegram bot
python store/bot.py &
```

### 5.8 Redirects After Login (next parameter)

When using a custom login view, ensure the `next` parameter is preserved from `GET` to `POST`.

**1. View Logic:**
```python
def user_login(request):
    if request.method == 'POST':
        # ... authenticate ...
        if user:
            login(request, user)
            # Check POST first (hidden input), then GET (query string)
            next_url = request.POST.get('next') or request.GET.get('next')
            return redirect(next_url) if next_url else redirect('home')
    return render(request, 'login.html')
```

**2. Template Form:**
Include a hidden input in the login form to carry the `next` value into the `POST` request.
```html
<form method="post">
  {% csrf_token %}
  <input type="hidden" name="next" value="{{ request.GET.next|default:'' }}">
  <!-- ... fields ... -->
</form>
```

**3. JavaScript Redirects:**
When triggering a login redirect from JS (e.g., an AJAX order button), manually append the `next` parameter.
```javascript
function loginAndReturn() {
    const nextPath = window.location.pathname + window.location.search;
    window.location.href = `/login/?next=${encodeURIComponent(nextPath)}`;
}
```

### 5.9 Dynamic Count Annotations & Foreign Keys

When annotating count of related objects in views (e.g. `ProductType` categories with count of `Product`), use the related_name defined on the `ForeignKey` (e.g. `related_name="products"`):

```python
from django.db.models import Count
product_types = ProductType.objects.annotate(
    product_count=Count('products')  # ← 'products' matching related_name="products" on Product.type
).filter(product_count__gt=0).order_by('-product_count')[:8]
```

### 5.10 Atomic Increments for Analytics/Views

Avoid read-then-save race conditions when updating counter fields (e.g. `views_count`):

```python
from django.db.models import F

# Atomic SQL UPDATE product SET views_count = views_count + 1 WHERE id = ...
Product.objects.filter(pk=product.pk).update(views_count=F('views_count') + 1)
```

### 5.11 Celery Circular Import Pitfall

If a top-level `celery.py` exists directly in the Django project root (e.g. `website/celery.py` alongside `manage.py`), `from celery import Celery` inside `website/website/celery.py` or `website/celery.py` will try to import the local file rather than the installed `celery` package.

**Symptom:**
`ImportError: cannot import name 'Celery' from partially initialized module 'celery' (most likely due to a circular import) (/path/to/celery.py)`

**Fix:**
Rename or remove root-level `celery.py` so Celery imports resolve to the package in `site-packages`. The Celery app configuration should reside inside the project package directory (`website/website/celery.py`).

---

## React Admin Panel + Unified JWT+Session Login

See `references/jwt-session-unified-login.md` for full code. Also see `references/render-free-deployment.md` for Render free tier deployment setup. Key pitfalls:

- **`@api_view` doesn't propagate session cookie** — DRF wraps the response and middleware `Set-Cookie` never fires. Use plain `@csrf_exempt` Django view for any endpoint that must set `sessionid`.
- **`CORS_ALLOW_ALL_ORIGINS=True` + `withCredentials`** — browsers reject cookies when origin is wildcard. Switch to explicit `CORS_ALLOWED_ORIGINS` list + `CORS_ALLOW_CREDENTIALS = True`.
- **axios login call must use the `api` instance** — `axios.post(...)` direct bypasses `withCredentials: true` set on the instance. Always `api.post('/admin-login/', ...)`.
- **Verify both cookies in response**: `curl -v ... | grep Set-Cookie` must show both `csrftoken` AND `sessionid`. If only `csrftoken`, session isn't being set.
- **gunicorn nohup (no systemd)**: reload with `kill -HUP $(pgrep -f 'gunicorn website.wsgi' | head -1)`. Never `systemctl restart gunicorn` — no service file exists.

## Pitfalls
- **Lost 'next' parameter**: Occurs if the login form `action` URL doesn't include the query string OR if the view only checks `request.GET`. Always check both `POST` and `GET`.
- **405 on order redirect**: If an order endpoint only allows `POST`, a direct redirect (which is `GET`) will fail. Redirect the user back to the product page instead, where they can click the button again.
