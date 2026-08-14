# RTL Frontend + LTR Admin (Unfold) — Design Pattern

## Context

The shop frontend must be full RTL (right-to-left) for Persian users: header, product grid, footer, everything. The Unfold admin panel must preserve its original LTR layout (sidebar on the left, top bar on the left) while rendering Persian text (product names, form labels) as right-aligned.

## Approach

### 1. Frontend: Full RTL in `base.html`

```html
<!DOCTYPE html>
<html lang="fa" dir="rtl">
```

This applies RTL to the entire frontend. Tailwind utilities work correctly with `dir="rtl"` — `left-*`/`right-*` swap meaning. Prefer `start-*`/`end-*` utilities when available.

Font: `Vazirmatn` (from Google Fonts, works inside Iran). Load via:
```html
<link href="https://fonts.googleapis.com/css2?family=Vazirmatn:wght@300;400;500;600;700;800&display=swap" rel="stylesheet">
```

### 2. Admin: Unfold `EXTRA_STYLES` config override (CORRECT)

**🚨 CRITICAL: Do NOT use `STYLES`. There is NO `STYLES` key in Unfold's config. Using it causes HTTP 500 on every admin page.**

The correct key is `EXTRA_STYLES` (for inline CSS strings) or `EXTRA_CSS` (for a URL to a stylesheet file).

#### Option A: Inline with `EXTRA_STYLES` (simpler)

```python
UNFOLD = {
    ...
    "EXTRA_STYLES": [
        # Make admin text RTL (form labels, product names, table cells)
        "body, input, textarea, select, option, .form-row label, "
        ".field-box label, p, h1, h2, h3, th, td, div { "
        "  text-align: right !important; direction: rtl !important; "
        "}",

        # Keep sidebar LTR (layout intact, user sees it on the left)
        ".sidebar, .nav-sidebar, .sidebar * { "
        "  text-align: left !important; direction: ltr !important; "
        "}",

        # Keep nav bar and action buttons centered/LTR
        ".navbar, .breadcrumbs, .module caption, .button, "
        "input[type=submit], .object-tools { "
        "  text-align: center !important; direction: ltr !important; "
        "}",

        # Form labels: float right so the label is to the right of the input
        ".aligned label, .form-row label { "
        "  float: right !important; "
        "  padding-left: 10px !important; "
        "  padding-right: 0 !important; "
        "}",

        # Text inputs: RTL direction so Persian text reads correctly
        ".vTextField, .vLargeTextField, textarea, input { "
        "  direction: rtl !important; text-align: right !important; "
        "}",
    ],
    ...
}
```

#### Option B: External file with `EXTRA_CSS` (cleaner)

Create `store/static/unfold/css/rtl-overrides.css` with the same CSS, then:

```python
UNFOLD = {
    ...
    "EXTRA_CSS": [
        "/static/unfold/css/rtl-overrides.css",
    ],
    ...
}
```

Then run `python manage.py collectstatic --noinput`.

### 3. Important gotchas

- **`!important` is required**: Unfold's own CSS has high specificity. Without `!important`, the overrides won't stick.
- **Sidebar icons**: If icons flip backwards (e.g., arrow points left instead of right), add `.sidebar .icon { transform: scaleX(-1); }`.
- **Date/time pickers**: These may break under RTL. Test carefully. If broken, add `.datetimeshortcuts { direction: ltr; }` to exclude them.
- **Action buttons in list view**: The "action" dropdown and "Go" button may float the wrong way. Add `.actions { direction: ltr; float: left; }` to keep the actions bar aligned left.
- **After changing UNFOLD config**, run `python manage.py collectstatic --noinput` and restart Gunicorn for changes to take effect.
- **`UNFOLD["STYLES"]` causes 500**: There is no `STYLES` key in django-unfold. Using it silenty crashes Django's config validation and every admin page returns HTTP 500 with a blank white page. Python/kombu log won't show the error — check Gunicorn access log for 500 status on `/admin/` and the settings.py file for `STYLES`.

## Debugging Admin 500s

```bash
# Quick check
curl -s -o /dev/null -w "%{http_code}" http://localhost:8000/admin/

# If 500, check logs
tail -5 /tmp/gunicorn.log | grep " 500 "

# Probable causes (in order):
# 1. UNFOLD config has invalid key (like "STYLES")
# 2. Missing Pillow after collectstatic
# 3. Template syntax error in admin templates
```

## When to use this

- User wants "RTL site for Persian but LTR admin layout" (they explicitly said sidebar and toolbar should stay on the left).
- User only complained about admin being "full RTL" (whole layout swapped).
- Alternative: Setting `LANGUAGE_CODE = 'fa'` in Django triggers automatic RTL in the default admin, but Unfold may or may not respect it properly. The EXTRA_STYLES approach gives manual control.
