# Django Shop — Implementation Patterns (from مهگل رزین build)

Extracted design-system patterns from a production Django shop with RTL frontend + LTR Unfold admin + Telegram bot + Celery. Pair with `SKILL.md` for the full design methodology.

## Wishlist / Favorites (AJAX Toggle)

```python
# Model
class Wishlist(models.Model):
    user = models.ForeignKey(User, on_delete=models.CASCADE, related_name="wishlist")
    product = models.ForeignKey(Product, on_delete=models.CASCADE, related_name="wishlisted_by")
    class Meta:
        unique_together = ("user", "product")

# View — @csrf_exempt IS REQUIRED for fetch() POST
@login_required
@csrf_exempt
def toggle_wishlist(request):
    ...  # JsonResponse({'wishlisted': True/False})

# Pass wishlisted_codes to template
wishlisted_codes = set(Wishlist.objects.filter(user=request.user).values_list("product__code", flat=True))
```

## Auth-State-Aware Navigation

```django
{% if user.is_authenticated %}
  {% if user.is_superuser or user.is_staff %}
    <a href="/admin/">پنل ادمین</a>
  {% endif %}
  <a href="{% url 'store:user_logout' %}">خروج</a>
{% else %}
  <a href="{% url 'store:user_login' %}">ورود</a>
  <a href="{% url 'store:user_signup' %}">ثبت نام</a>
{% endif %}
```

## Guarded Send-to-Bot Button

Anonymous user clicks "سفارش" → redirect to login, not JS error or silent failure.

```javascript
function sendToBot(code) {
  {% if user.is_authenticated %}
    // confirm + fetch(/order/CODE/) → redirect to /bot_order_sent
  {% else %}
    window.location.href = "{% url 'store:user_login' %}";
  {% endif %}
}
```

## Static Page Pattern

```python
def about(request):
    return render(request, 'store/about.html')
# urls.py: path("about/", views.about, name="about")
```

Template: extends `base.html`, sections: Hero → Story → Values → Stats → Process → CTA. Same spacing scale (`py-12 md:py-16`), same color palette (`violet-600`, `violet-50`, `gray-900`).

## Content Management — Two Layers

### Layer 1: Single-row SiteSetting (global texts)
- Model: `SiteSetting` with hero_title, hero_subtitle, etc.
- Context processor: `store.context_processors.site_settings`
- Admin: singleton (disable add/delete)
- Use `|safe` in template if HTML is expected

### Layer 2: Full Page model (dynamic HTML pages)
- Model: `Page` with title, slug, content (HTML), image, status
- TinyMCE CDN-based editor (no pip install)
- View: `get_object_or_404(Page, slug=slug, status="published")`
- URL: `/page/<slug>/`
- Status: draft/published — only published pages are accessible

## Product Auto-Code — Filter by Type

```python
# BAD — range-based (can cross type boundaries)
Product.objects.filter(code__gte=start, code__lt=start + 10000)

# GOOD — filter by type
Product.objects.filter(type=self.type).aggregate(m=Max("code"))["m"]
```

## MEDIA_ROOT Must Match Nginx Path

```python
# settings.py
MEDIA_ROOT = "/var/www/resin-media"
MEDIA_URL = "/media/"

# Nginx
location /media/ { alias /var/www/resin-media/; }
```

Copy files once: `cp -r /old/path/media/ /var/www/resin-media/`

## Login Loop Fix (HTTP, no HTTPS)

```python
CSRF_COOKIE_SECURE = False
SESSION_COOKIE_SECURE = False
SESSION_COOKIE_HTTPONLY = True
SESSION_EXPIRE_AT_BROWSER_CLOSE = True
python manage.py clearsessions  # invalidate stale sessions
```

Test in Incognito mode.

## Low-RAM OOM Workaround

When RAM < 1GB, Redis + Celery causes OOM fork bombs.  
**Immediate:** `systemctl stop redis; pkill -f celery;` reduce gunicorn to 1 worker.  
**Long-term cron replacement** for promotion queue:

```bash
#!/bin/bash
cd /project/website && source ../venv/bin/activate
python -c "
import os; os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'website.settings')
import django; django.setup()
from store.tasks import process_promotion_queue
process_promotion_queue()
"
```

Add to crontab: `0 10 * * * /path/to/script.sh`

## Admin Unfold RTL/LTR Split

- `LANGUAGE_CODE = 'en-us'` in settings → keeps admin LTR
- `dir="rtl"` in `base.html` → keeps site RTL
- Persian text renders correctly in LTR mode — only layout changes
- Do NOT use `UNFOLD["STYLES"]` — crashes Django config validation
