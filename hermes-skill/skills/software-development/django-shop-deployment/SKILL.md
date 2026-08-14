---
name: django-shop-deployment
description: Deploy a Django-based e-commerce shop integrated with a Telegram bot (python-telegram-bot) and Celery task queue. Covers Docker Compose setup, Nginx reverse proxy, PostgreSQL migration, secret management, and bot-as-service patterns.
category: software-development
tags:
  - django
  - telegram-bot
  - celery
  - docker
  - deployment
  - nginx
  - postgresql
---

# Django Shop + Telegram Bot Deployment

Deploy a Django shop with a Telegram admin/user bot and Celery-powered scheduled posting to a Telegram channel. This skill covers the entire stack from local dev to production on a Linux server.

## Architecture Overview

```
┌─────────────────────────────────────────────────┐
│                   Nginx                          │
│  (static/media server + reverse proxy to Django) │
└────────┬──────────────────────────┬──────────────┘
         │                          │
┌────────▼────────┐    ┌───────────▼────────────┐
│   Django/Gunicorn│    │  Telegram Bot (python- │
│   (port 8000)    │    │  telegram-bot, port X) │
│   - REST API     │    │  - Admin CRUD          │
│   - Admin panel  │    │  - User browse/order   │
│   - Templates    │    │  - Channel posting     │
└────────┬────────┘    └───────────┬────────────┘
         │                         │
┌────────▼─────────────────────────▼────────────┐
│              PostgreSQL                        │
│  (shared database between Django and Bot)      │
└───────────────────────────────────────────────┘
┌───────────────────────────────────────────────┐
│              Redis                             │
│  (Celery broker + result backend)             │
└───────────────────────────────────────────────┘
┌───────────────────────────────────────────────┐
│           Celery Worker                        │
│  - Promotion queue processing                 │
│  - Scheduled tasks (Celery Beat)              │
└───────────────────────────────────────────────┘
```

## Pre-Deployment Checklist

Before deploying, fix these common issues in a Django+Bot project:

### 🔴 Critical Fixes

```python
# settings.py — NEVER hardcode secrets
SECRET_KEY = os.environ.get("DJANGO_SECRET_KEY", "")
TELEGRAM_BOT_TOKEN = os.environ.get("TELEGRAM_BOT_TOKEN", "")

# DATABASES — switch from SQLite to PostgreSQL
DATABASES = {
    "default": {
        "ENGINE": "django.db.backends.postgresql",
        "NAME": os.environ.get("DB_NAME", "shop"),
        "USER": os.environ.get("DB_USER", "shop"),
        "PASSWORD": os.environ.get("DB_PASSWORD", ""),
        "HOST": os.environ.get("DB_HOST", "db"),
        "PORT": os.environ.get("DB_PORT", "5432"),
    }
}

# CELERY — set Redis URL
CELERY_BROKER_URL = os.environ.get("CELERY_BROKER_URL", "redis://redis:6379/0")
CELERY_RESULT_BACKEND = os.environ.get("CELERY_RESULT_BACKEND", "redis://redis:6379/1")
```

### 🐛 Common Bugs Found in Real Projects

| Bug | Fix |
|-----|-----|
| `self.stauts` instead of `self.status` in model method | Rename to `self.status` |
| `SECRET_KEY` hardcoded in settings.py | Move to environment variable |
| `TELEGRAM_BOT_TOKEN` hardcoded in settings.py | Move to environment variable |
| `DEBUG=True` in production | Set to `os.environ.get("DJANGO_DEBUG", "False") == "True"` |
| No `requirements.txt` | Create with exact versions |
| `KENDO_BOT_TOKEN` or `Token` variable name conflict with `TELEGRAM_BOT_TOKEN` | Keep one canonical env var name and alias it |
| Bot references `Token` in `__main__` but `TELEGRAM_BOT_TOKEN` is imported from settings | Unify variable name throughout bot.py |
| `settings.py` has no `.env` loader | Add `python-dotenv` + `load_dotenv(BASE_DIR / ".env")` |
| `celerybeat-schedule` committed to git | Add to `.gitignore` |
📍 **CoinGecko gold API returns unreliable data**: CoinGecko's `gold` ticker returns per-gram values that are often zero or stale. Use **GoldAPI** (`api.gold-api.com/price/XAU`) for real XAU prices instead. See `references/crypto-prices-debugging.md` for full debugging session.
| **Product codes collide across types** | Filter by `type=self.type` in `save()`, not by number range |
| **UNFOLD config `STYLES` key causes HTTP 500** | `STYLES` is NOT a valid Unfold key. Using it crashes Django config validation silently. Use `EXTRA_STYLES` (inline CSS strings) or `EXTRA_CSS` (stylesheet URL path). See `references/rtl-site-ltr-admin-unfold.md` |
| **Nginx `/root/` permissions cause 403** | Never point Nginx alias to `/root/`. Copy static/media to `/var/www/resin-*/` and `chown -R www-data:www-data` before configuring nginx |
| **Low-RAM OOM (exit code 137)** | `dmesg | grep -i oom` to confirm. Immediate fix: kill redis + celery, reduce gunicorn to 1 worker. Long-term: cron-based Celery replacement. See `references/server-migration-low-ram.md` |
| **Site/server migration** | Transfer exactly 4 things: `db.sqlite3`, `website/media/`, `website/.env`, `~/.hermes/`. New server needs: nginx + venv + pip install + gunicorn + bot. See `references/server-migration-low-ram.md` |
| **Admin login loop (302 never passes)** | On HTTP (no HTTPS), set `CSRF_COOKIE_SECURE=False` + `SESSION_COOKIE_SECURE=False` + run `clearsessions`. Test in Incognito. |
| **ESP32-S3 LEDC API changes (Arduino Core 3.x)** | `ledcSetup()` + `ledcAttachPin()` → `ledcAttach(pin, freq, resolution)`; `ledcWrite(channel, duty)` → `ledcWrite(pin, duty)`. Use `ledcAttachChannel()` for explicit channel. |
| **ESP32-S3 strapping pins conflict with LCD+PWM** | GPIO 15 is a strapping pin; using it for LCD + servo on adjacent pins causes random hangs. Move LCD to GPIO 1/2; servo to GPIO 10/3. See `references/esp32-s3-servo-pitfalls.md`. |
| **React Admin Dashboard Build Stale (`/dashboard/`)** | After updating React code in `admin-frontend/src/App.jsx`, run `npm run build` inside `admin-frontend/` and copy `dist/*` to `website/staticfiles/dashboard/`. Clear old `index-*.js` files to prevent Nginx from serving stale build chunks containing hardcoded URLs. |
| **`views_count` & Dynamic Featured Product** | Add `views_count` to Product model, increment atomically with `F('views_count') + 1` in `product_detail`, and query featured product with `.order_by('-views_count', '-id').first()`. |
| **Dynamic Product Categories Ordering** | Group ProductType by product count using `ProductType.objects.annotate(count=Count('product')).filter(count__gt=0).order_by('-count')`. |
- **primary_image not appearing in templates** | Django templates **cannot chain** `.filter(...).first()`. Writing `item.product.images.filter(is_primary=True).first()` in a template silently fails (returns nothing). **Fix:** pre-compute in the view: `item.primary_image = item.product.images.filter(is_primary=True).first()`, then use `{{ item.primary_image.image.url }}` in template. |
| **LCD + Servo conflict on GPIO 15** | GPIO 15 is a strapping pin on ESP32-S3; using it for LCD D6 + Servo on GPIO 13 causes PWM conflict. Move LCD D6/D7 to GPIO 1/2; servo to GPIO 10/3. |
| **Product codes collide across types** | Filter by `type=self.type` in `save()`, not by number range |
| **UNFOLD config `STYLES` key causes HTTP 500** | `STYLES` is NOT a valid Unfold key. Using it crashes Django config validation silently. Use `EXTRA_STYLES` (inline CSS strings) or `EXTRA_CSS` (stylesheet URL path). See `references/rtl-site-ltr-admin-unfold.md` |
| **Nginx `/root/` permissions cause 403** | Never point Nginx alias to `/root/`. Copy static/media to `/var/www/resin-*/` and `chown -R www-data:www-data` before configuring nginx |
| **Low-RAM OOM (exit code 137)** | `dmesg | grep -i oom` to confirm. Immediate fix: kill redis + celery, reduce gunicorn to 1 worker. Long-term: cron-based Celery replacement. See `references/server-migration-low-ram.md` |
| **Site/server migration** | Transfer exactly 4 things: `db.sqlite3`, `website/media/`, `website/.env`, `~/.hermes/`. New server needs: nginx + venv + pip install + gunicorn + bot. See `references/server-migration-low-ram.md` |
| **Admin login loop (302 never passes)** | On HTTP (no HTTPS), set `CSRF_COOKIE_SECURE=False` + `SESSION_COOKIE_SECURE=False` + run `clearsessions`. Test in Incognito. |
| **CTA `<p>` tag layout breakage** | Persian content + missing `</p>` on same line as block element → layout collapses. Always close `<p>` on its own line before block elements. |
| **`MEDIA_URL` context processor missing** | `{{ MEDIA_URL }}` empty in templates → logo `src="/media/logo.jpg"` broken. Add `'django.template.context_processors.media',` to `context_processors` in `settings.py`. |
| **SiteSettings CMS Expansion** | Expand `SiteSetting` model with fields for About/Contact/Footer/Home. Inject via global `site_settings` context processor. |

### 📦 Required Dependencies (`requirements.txt`)

```
django==5.2
gunicorn==23.0
psycopg2-binary==2.9
python-telegram-bot==21.0
celery==5.4
redis==5.0
django-unfold  # Django admin theme
Pillow  # image handling
requests  # for bot HTTP calls
jq  # (system dependency, for scripts)
```

## Wishlist / Favorites (AJAX Toggle)

Favorites is a **stateful feature** — requires DB + server endpoint + client JS toggle.

### Model

```python
class Wishlist(models.Model):
    user = models.ForeignKey(User, on_delete=models.CASCADE, related_name="wishlist")
    product = models.ForeignKey(Product, on_delete=models.CASCADE, related_name="wishlisted_by")
    created_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        unique_together = ("user", "product")
        ordering = ("-created_at",)
```
Run `makemigrations` + `migrate`.

### AJAX Endpoint (view)

```python
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt

@login_required
@csrf_exempt  # ← needed because client fetch() sends POST without CSRF header
def toggle_wishlist(request):
    if request.method != 'POST':
        return JsonResponse({'error': 'POST required'}, status=405)
    code = request.POST.get('code')
    product = get_object_or_404(Product, code=code)
    wish, created = Wishlist.objects.get_or_create(user=request.user, product=product)
    if not created:
        wish.delete()
        return JsonResponse({'status': 'removed', 'wishlisted': False})
    return JsonResponse({'status': 'added', 'wishlisted': True})
```

### Wishlist Page (view)

```python
@login_required
def wishlist_view(request):
    items = Wishlist.objects.filter(user=request.user).select_related('product__type')
    products = []
    for item in items:
        p = item.product
        p.primary_image = p.images.filter(is_primary=True).first()
        products.append(p)
    return render(request, 'store/wishlist.html', {'products': products})
```

### Heart Button on Product Cards

Each product card gets an absolute-positioned heart button:

```django
<button onclick="toggleWishlist(event, {{ p.code }})"
        class="wishlist-btn absolute top-3 left-3 z-10 w-9 h-9 rounded-full bg-white/80 flex items-center justify-center shadow-sm"
        data-code="{{ p.code }}">
  <svg class="w-5 h-5 {% if p.code in wishlisted_codes %}text-rose-500 fill-current{% else %}text-gray-400{% endif %}"
       fill="{% if p.code in wishlisted_codes %}currentColor{% else %}none{% endif %}"
       stroke="currentColor" viewBox="0 0 24 24">
    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4.318 6.318a4.5 4.5 0 000 6.364L12 20.364l7.682-7.682a4.5 4.5 0 00-6.364-6.364L12 7.636l-1.318-1.318a4.5 4.5 0 00-6.364 0z"/>
  </svg>
</button>
```

### JS (in global `base.html`, reused across pages)

```javascript
function toggleWishlist(event, code) {
  event.preventDefault();
  event.stopPropagation();
  const isAuth = {{ user.is_authenticated|yesno:"true,false" }};
  if (!isAuth) {                    // ← redirect unauthenticated users to login
    window.location.href = "{% url 'store:user_login' %}";
    return;
  }
  fetch('/wishlist/toggle/', {
    method: 'POST',
    headers: { 'X-CSRFToken': '{{ csrf_token }}' },
    body: 'code=' + code
  })
  .then(r => r.json())
  .then(data => {
    const svg = document.querySelector(`.wishlist-btn[data-code="${code}"] svg`);
    if (svg) {
      if (data.wishlisted)
        svg.classList.replace('text-gray-400', 'text-rose-500'); svg.classList.add('fill-current');
      else
        svg.classList.replace('text-rose-500', 'text-gray-400'); svg.classList.remove('fill-current');
    }
  });
}
```

### Pass wishlisted_codes to the List Page (view)

```python
def product_list(request):
    ...
    wishlisted_codes = set()
    if request.user.is_authenticated:
        wishlisted_codes = set(
            Wishlist.objects.filter(user=request.user).values_list("product__code", flat=True)
        )
    return render(request, "store/product_list.html", {
        ...
        "wishlisted_codes": wishlisted_codes,
    })
```

### Key Insight — `@csrf_exempt` on AJAX Endpoint

`fetch()` POST from a plain JS script (no `<form>`) doesn't include the CSRF cookie reliably. The cleanest fix is `@csrf_exempt` on the toggle view. Without it, you get a **403** when the endpoint is called from client-side JS.

### "Add All to Cart" Button

See `references/wishlist-add-all-to-cart.md` for the button + `/wishlist/add-all/` endpoint that moves all wishlist items into the cart in one click.

### Wishlist Page — Also Gets AJAX + Add-All

The wishlist page (`store/wishlist.html`) should match the product page UX:
- Each product card has a `+ سبد` button calling `addToCart(code, this)` (AJAX, no page reload)
- A header button "افزودن همه به سبد خرید" calls `addAllWishlistToCart()` (POST to `/wishlist/add-all/`) then redirects to the cart page
- See `references/wishlist-add-all-to-cart.md` for the full implementation

## Mobile Menu — Auth-Aware Nav

The hamburger menu (`#mobile-menu`) must mirror the desktop header's auth state:

```django
<div id="mobile-menu" class="hidden md:hidden ...">
  <a href="/">خانه</a>
  <a href="{% url 'store:product_list' %}">محصولات</a>
  <a href="{% url 'store:about' %}">درباره ما</a>
  <hr>
  {% if user.is_authenticated %}
    <a href="{% url 'store:wishlist' %}">علاقه‌مندی‌ها</a>
    {% if user.is_superuser or user.is_staff %}
      <a href="/admin/">پنل ادمین</a>
    {% endif %}
    <a href="{% url 'store:user_logout' %}">خروج</a>
  {% else %}
    <a href="{% url 'store:user_login' %}">ورود</a>
    <a href="{% url 'store:user_signup' %}">ثبت نام</a>
  {% endif %}
</div>
```

## Static Page Pattern (About, Contact, etc.)

Simple static pages need only a view + template:

```python
def about(request):
    return render(request, 'store/about.html')
```

```python
# urls.py
path("about/", views.about, name="about"),
```

### Template structure

- Extends `store/base.html` (header + footer already there)
- Hero section with `bg-gradient-to-b from-violet-50`
- Max width `max-w-7xl mx-auto`
- Sections separated by spacing (`py-12 md:py-16`)
- No custom CSS — uses Tailwind classes throughout
- CTA at bottom reusing the `bg-violet-700` button style

## Telegram Order Flow — Redirect to Bot + Admin Notification

The order flow now **redirects the user to Telegram** via deep link instead of a server-side POST. The bot handles confirmation and admin notification.

### Frontend (base.html `sendToBot`)

```javascript
function sendToBot(code) {
  {% if user.is_authenticated %}
  if (confirm(`آیا می‌خواهید این محصول را در تلگرام سفارش دهید؟`)) {
    window.location.href = `https://t.me/Mahgolresinadminbot?start=order_${code}`;
  }
  {% else %}
  const currentProductPath = `/product/${code}/`;
  window.location.href = `{% url 'store:user_login' %}?next=${encodeURIComponent(currentProductPath)}`;
  {% endif %}
}
```

**Key rules:**
- Use `window.location.href` NOT `window.open(..., '_blank')` — Brave/Chrome block popups.
- When user is not authenticated, redirect to login with `?next=/product/<code>/` — after login they return to the product page and can try again.
- No POST to `/order/<code>/` needed anymore — the bot handles everything.

### Login Redirect Flow (preserve `next` through POST)

```python
# views.py — CRITICAL: check POST first, then GET
def user_login(request):
    if request.method == 'POST':
        next_url = request.POST.get('next') or request.GET.get('next')
        if next_url:
            return redirect(next_url)
        return redirect('store:product_list')
```

```html
<!-- login.html form — preserve next through both action URL and hidden input -->
<form method="post" action="{% url 'store:user_login' %}{% if request.GET.next %}?next={{ request.GET.next|urlencode }}{% endif %}" class="space-y-5">
  {% csrf_token %}
  <input type="hidden" name="next" value="{{ request.GET.next|default:'' }}">
```

### Bot start handler — parse `order_<code>` deep link

```python
async def start_command_handler(update: Update, context: ContextTypes.DEFAULT_TYPE):
    # Save Telegram user profile first (always, even without order code)
    telegram_user_id = update.effective_user.id
    await sync_to_async(save_user_profile)(user_id=telegram_user_id, ...)

    args = context.args
    if args and args[0].startswith("order_"):
        code = int(args[0][6:])
        product = await sync_to_async(Product.objects.select_related("type").filter(code=code).first)()
        if not product:
            await update.message.reply_text("❌ محصول یافت نشد.")
            return
        # Build product card + inline confirm/cancel buttons
        ...
```

### Order confirm handler — notify admin + create queue item

```python
async def order_confirm_handler(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    if not query: return
    await query.answer()

    code = int(query.data.split(":")[1])
    product = await sync_to_async(Product.objects.filter(code=code).first)()
    await sync_to_async(PromotionQueueItem.objects.create)(product=product)

    # Admin notification — MUST use NUMERIC chat_id, NOT @username
    user_data = update.effective_user
    msg_admin = f"✅ *سفارش جدید تایید شد*\\n\\n"
    msg_admin += f"👤 *مشتری:* [{user_data.full_name}](tg://user?id={user_data.id})\\n"
    msg_admin += f"🆔 *آیدی:* `{user_data.id}`\\n"
    if user_data.username:
        msg_admin += f"📱 *یوزرنیم:* @{user_data.username}\\n"
    msg_admin += f"\\n🔹 *محصول:* {product.name}\\n🔹 *کد:* #{product.code}\\n🔹 *قیمت:* {product.price:,} تومان\\n"
    
    # Always use numeric chat_id, never @username, for reliable delivery
    await context.bot.send_message(chat_id=NUMERIC_ADMIN_ID, text=msg_admin, parse_mode="HTML")
```

### Register handlers in bot.py

```python
app.add_handler(CallbackQueryHandler(order_confirm_handler, pattern="^order_confirm:"))
app.add_handler(CallbackQueryHandler(order_cancel_handler, pattern="^order_cancel$"))
```

### Profile model for Telegram users

```python
class Profile(models.Model):
    user = models.OneToOneField(User, on_delete=models.CASCADE, null=True, blank=True)
    telegram_id = models.BigIntegerField(null=True, blank=True, unique=True)
    username = models.CharField(max_length=100, blank=True, null=True)
    first_name = models.CharField(max_length=100, blank=True, null=True)
    last_name = models.CharField(max_length=100, blank=True, null=True)
    is_admin = models.BooleanField(default=False)
```

### Save user profile utility (bot.py)

```python
def save_user_profile(user_id, username, first_name, last_name):
    profile, created = Profile.objects.get_or_create(telegram_id=user_id)
    profile.username = username
    profile.first_name = first_name
    profile.last_name = last_name
    profile.save()
```

### Pitfalls

1. **Using only `request.GET.get('next')`** — breaks because `next` is sent via POST body. Must check POST first: `request.POST.get('next') or request.GET.get('next')`.
2. **Missing hidden input in login form** — form won't carry `next` through POST. Add `<input type=\"hidden\" name=\"next\" value=\"{{ request.GET.next|default:'' }}\">` to the form.
3. **`action` URL without `?next=`** — if the form `action` loses the query string, LOGIN view reads `next` from POST body only. Best practice: keep `?next=` in the action URL AND hidden input.
4. **Browser cache of old JS** — after fixing, user must Hard Refresh (Ctrl+Shift+R).
5. **`window.open()` blocked** — Brave/Chrome block popups by default. Use `window.location.href`.
6. **Bot crashes on Telegram API timeout** — Iran ISPs throttle api.telegram.org. Wrap bot in systemd with `Restart=always`.
7. **`update.callback_query` can be `None`** — Always guard with `if not query: return`.
8. **`PromotionQueueItem` import missing** — Must import in bot.py: `from store.models import ..., PromotionQueueItem`.
8. **Bot profile schema change** — needs `makemigrations` + `migrate` when adding Profile fields.
9. **Cart + CartItem usage** — See `references/cart-implementation.md` for full pattern including `get_or_create_cart`, guest vs authenticated, token generation, bot handler for `start=cart_<token>`, `cart_confirm_handler`, and `checkout_to_telegram`. Note that admin must be called with `chown -R www-data:www-data /var/www/resin-media` after all migrations.
9. **Telegram deep link 64-char limit** — `order_<code>` is fine. Never put long data in deep link.
10. **Admin message parse mode** — Markdown in Telegram is strict. Prefer `parse_mode="HTML"` for reliability with special characters in product names.
11. **`chat_id="@username"` fails silently** — `send_message(chat_id="@username")` is silently dropped if the bot has never received a `/start` from that user. Always use **numeric chat_id** (e.g. `95851963`) for admin notifications.
12. **`.env` reload** — After updating `.env`, restart BOTH gunicorn AND bot.py. Django caches env vars at process start.

See `references/login-redirect-flow.md` for the full implementation breakdown with all edge cases.

## TinyMCE WYSIWYG Editor in Admin (CDN — No pip install)

### 1. Override the admin template

Create `templates/admin/store/page/change_form.html`:

```django
{% extends "admin/change_form.html" %}
{% load static %}

{% block admin_change_form_document_ready %}
{{ block.super }}
<script>
(function() {
  if (typeof tinymce === 'undefined') return;
  const contentField = document.querySelector('#id_content');
  if (!contentField) return;
  tinymce.init({
    target: contentField,
    height: 500,
    menubar: true,
    plugins: 'lists link image table code help',
    toolbar: 'undo redo | bold italic underline | fontselect fontsizeselect | alignleft aligncenter alignright | outdent indent | numlist bullist | image link | removeformat | code',
    directionality: 'rtl',
    promotion: false,
    setup: function (editor) {
      editor.on('init', function () {
        editor.getContainer().style.direction = 'rtl';
      });
    },
  });
})();
</script>
{% endblock %}
```

### 2. Register Media + context in admin.py

```python
@admin.register(Page)
class PageAdmin(admin.ModelAdmin):
    # ... existing config ...

    class Media:
        js = ('https://cdn.jsdelivr.net/npm/tinymce@6/tinymce.min.js',)

    def changeform_view(self, request, object_id=None, form_url='', extra_context=None):
        extra_context = extra_context or {}
        extra_context['use_tinymce'] = True
        return super().changeform_view(request, object_id, form_url, extra_context)
```

### 3. Register the template directory in settings.py

```python
# website/settings.py — TEMPLATES list
'DIRS': [BASE_DIR / 'templates'],
```

**Result:** Admin edits Page content in a visual editor (bold, italic, lists, images, tables) — no HTML literacy needed.

---

### CMS/Live Edit & Debug Toolbar

For instant visual content management on the storefront (inline text editing saved directly to the database), implement the **Live Debug/Edit** pattern. This bypasses rigid HTML builders and allows the admin to edit plain text strings on the live website.

#### 1. Models & Configuration

Live settings can be persisted in the `SiteSetting` model under a centralized JSON dictionary:

```python
# store/models.py
class SiteSetting(models.Model):
    # ... existing fields ...
    description = models.TextField(default="{}", blank=True, verbose_name="محتوای پویای صفحات (JSON)")
```

#### 2. Context Processor for Dynamic Layouts

Expose the JSON data along with the current page URL name dynamically to all templates:

```python
# store/context_processors.py
import json

def live_content(request):
    data = {}
    try:
        setting = SiteSetting.objects.first()
        if setting and setting.description and setting.description.startswith('{'):
            data = json.loads(setting.description)
    except Exception:
        pass
    
    # Determine the page key dynamically from resolve match
    view_name = 'home'
    try:
        if request.resolver_match:
            view_name = request.resolver_match.url_name or 'home'
    except Exception:
        pass
        
    return {
        'live_data': data,
        'current_page_name': view_name,
    }
```

Register this in `settings.py` TEMPLATES under `context_processors`.

#### 3. Frontend Editable Attribute `data-live-key`

Mark any editable text tag in any template with `data-live-key="element_unique_id"` and render it using the context processor's dictionary:

```django
<!-- store/templates/store/about.html -->
<h1 data-live-key="about_hero_title">
  {{ live_data.about.about_hero_title|default:"خلق زیبایی با هنر رزین" }}
</h1>
```

#### 4. The Live Save Backend Endpoint

Only authenticated staff/admins can write changes back to the JSON store:

```python
# store/views.py
import json as _json
from django.views.decorators.csrf import csrf_exempt

@csrf_exempt
def live_save(request):
    """Admin-only live content save endpoint."""
    if not request.user.is_authenticated or not request.user.is_staff:
        return JsonResponse({'error': 'forbidden'}, status=403)
    if request.method != 'POST':
        return JsonResponse({'error': 'POST required'}, status=405)
    try:
        data = _json.loads(request.body)
        page = data.get('page')    # e.g., 'home', 'about'
        key  = data.get('key')     # e.g., 'about_hero_title'
        value = data.get('value', '')
    except Exception:
        return JsonResponse({'error': 'invalid json'}, status=400)
        
    if not page or not key:
        return JsonResponse({'error': 'page and key required'}, status=400)

    setting = SiteSetting.objects.first() or SiteSetting.objects.create()
    try:
        live_data = _json.loads(setting.description) if setting.description.startswith('{') else {}
    except Exception:
        live_data = {}
        
    if page not in live_data:
        live_data[page] = {}
    live_data[page][key] = value
    setting.description = _json.dumps(live_data, ensure_ascii=False)
    setting.save()
    return JsonResponse({'status': 'ok', 'page': page, 'key': key})
```

#### 5. Admin Live Edit Overlay JavaScript (Inject in base.html)

Render the editing bar and enable inline editing/auto-save only if the user is staff/superuser:

```django
{% if user.is_staff or user.is_superuser %}
<div id="live-debug-toolbar" style="position:fixed;bottom:16px;left:16px;z-index:9999;display:flex;align-items:center;gap:8px;background:#140321;border:1px solid #6809A2;border-radius:999px;padding:8px 16px;box-shadow:0 4px 24px rgba(104,9,162,0.3);font-family:Vazirmatn,sans-serif;direction:rtl;">
  <span style="color:#9D09A2;font-size:10px;font-weight:900;">LIVE EDIT</span>
  <label style="display:flex;align-items:center;gap:6px;cursor:pointer;">
    <div id="live-toggle" onclick="toggleLiveDebug()" style="width:36px;height:20px;border-radius:10px;background:#333;position:relative;">
      <div id="live-knob" style="width:16px;height:16px;border-radius:50%;background:white;position:absolute;top:2px;right:2px;transition:right 0.2s;"></div>
    </div>
    <span id="live-label" style="color:#6B4E7A;font-size:10px;font-weight:700;">غیرفعال</span>
  </label>
  <span id="live-status" style="color:#6B4E7A;font-size:9px;"></span>
</div>

<script>
(function(){
  var active = false;
  var csrfToken = '{{ csrf_token }}';
  var pageEl = document.querySelector('main[data-page]');
  var pageName = pageEl ? pageEl.getAttribute('data-page') : 'home';
  var saveTimer = null;

  window.toggleLiveDebug = function() {
    active = !active;
    var toggle = document.getElementById('live-toggle');
    var knob = document.getElementById('live-knob');
    var label = document.getElementById('live-label');
    if (active) {
      toggle.style.background = '#6809A2';
      knob.style.right = '2px'; knob.style.left = '2px';
      label.style.color = '#a855f7'; label.textContent = 'فعال';
      enableEditing();
    } else {
      toggle.style.background = '#333';
      knob.style.right = '2px'; knob.style.left = '';
      label.style.color = '#6B4E7A'; label.textContent = 'غیرفعال';
      disableEditing();
    }
  };

  function enableEditing() {
    document.querySelectorAll('[data-live-key]').forEach(function(el) {
      el.setAttribute('contenteditable', 'true');
      el.style.outline = '2px dashed rgba(104,9,162,0.5)';
      el.style.borderRadius = '4px';
      el.addEventListener('input', onInput);
      el.addEventListener('blur', saveNow);
    });
  }

  function disableEditing() {
    document.querySelectorAll('[data-live-key]').forEach(function(el) {
      el.removeAttribute('contenteditable');
      el.style.outline = ''; el.style.borderRadius = '';
      el.removeEventListener('input', onInput);
      el.removeEventListener('blur', saveNow);
    });
  }

  function onInput(e) {
    clearTimeout(saveTimer);
    saveTimer = setTimeout(function() { saveElement(e.target); }, 800);
  }

  function saveNow(e) {
    clearTimeout(saveTimer);
    saveElement(e.target);
  }

  function saveElement(el) {
    var key = el.getAttribute('data-live-key');
    var value = el.innerText || el.textContent;
    var elPage = pageName;
    if (el.closest('footer')) {
      elPage = 'base';
    } else if (el.closest('[data-live-page]')) {
      elPage = el.closest('[data-live-page]').getAttribute('data-live-page');
    }
    fetch('/live-save/', {
      method: 'POST',
      headers: {'Content-Type': 'application/json', 'X-CSRFToken': csrfToken},
      body: JSON.stringify({page: elPage, key: key, value: value})
    })
    .then(r => r.json())
    .then(d => {
      if (d.status === 'ok') document.getElementById('live-status').textContent = 'ذخیره شد ✓';
    });
  }
})();
</script>
{% endif %}
```

#### 6. Pitfalls & Troubleshooting

- **Double-bracket strings in context processor**: Avoid unescaped or unparsed raw strings inside docstrings when editing dynamic scripts to prevent Python syntax errors that could crash Gunicorn/WSGI.
- **Scope resolution for shared elements**: Always ensure footer elements are saved under a global namespace (like `'base'`) rather than the current active page template, allowing changes to propagate site-wide immediately.

### CMS/Live Edit & Debug Toolbar

Two complementary layers cover different needs:

### Layer 1: Site-Wide Texts (SiteSetting Model)

For global snippets — hero, footer — that should be editable from one admin screen without touching HTML, use a **single‑row `SiteSetting` model** with a context processor.

### Model (`store/models.py`)

```python
class SiteSetting(models.Model):
    hero_title = models.CharField(max_length=200, default="محصولات دست‌ساز", verbose_name="عنوان هدر اصلی")
    hero_subtitle = models.TextField(max_length=500, default="...", verbose_name="زیرنویس هدر")
    hero_cta_text = models.CharField(max_length=100, default="مشاهده محصولات", verbose_name="متن دکمه هدر")
    footer_text = models.TextField(max_length=500, default="...", verbose_name="متن فوتر")
    about_title = models.CharField(max_length=200, verbose_name="عنوان درباره ما")
    about_text = models.TextField(max_length=2000, default="...", verbose_name="متن درباره ما")
    updated_at = models.DateTimeField(auto_now=True)

    class Meta:
        verbose_name = "تنظیمات سایت"
        verbose_name_plural = "تنظیمات سایت"

    def __str__(self):
        return "تنظیمات سایت"
```

### Admin — singleton enforcement

```python
@admin.register(SiteSetting)
class SiteSettingAdmin(admin.ModelAdmin):
    fieldsets = (
        ("هدر سایت", {"fields": ("hero_title", "hero_subtitle", "hero_cta_text")}),
        ("فوتر", {"fields": ("footer_text",)}),
        ("صفحه درباره ما", {"fields": ("about_title", "about_text")}),
    )
    def has_add_permission(self, request):
        return not SiteSetting.objects.exists()
    def has_delete_permission(self, request, obj=None):
        return False
```

### Context processor (`store/context_processors.py`)

```python
from .models import SiteSetting

def site_settings(request):
    s = SiteSetting.objects.first()
    return {"site_settings": s or SiteSetting()}
```

Register in `settings.py` → `TEMPLATES[0]["OPTIONS"]["context_processors"]` → add `"store.context_processors.site_settings"`.

**Pitfalls:**

* **Import in admin.py** — forgetting `from .models import SiteSetting` causes `NameError: name 'SiteSetting' is not defined`.
* **Seed on first deploy** — run `python manage.py shell -c "from store.models import SiteSetting; SiteSetting.objects.get_or_create()"` after migration.
* **`|safe` filter** — use `{{ site_settings.hero_title|safe }}` in templates only if the content should allow HTML. Omit `|safe` for plain text.

### Layer 2: Full HTML Pages (Page Model)

For dynamic pages (promotions, landing pages, seasonal content) that an admin should create and publish entirely through the admin panel without touching code, use a **storable-HTML `Page` model**.

#### Model (`store/models.py`)

```python
class Page(models.Model):
    STATUS_CHOICES = [
        ("draft", "پیش‌نویس"),
        ("published", "منتشر شده"),
    ]
    
    title = models.CharField(max_length=200, verbose_name="عنوان صفحه")
    slug = models.SlugField(max_length=200, unique=True, allow_unicode=True, verbose_name="لینک یکتا")
    content = models.TextField(verbose_name="محتوای صفحه (HTML)")
    image = models.ImageField(upload_to="pages/", blank=True, null=True, verbose_name="تصویر شاخص")
    status = models.CharField(max_length=20, choices=STATUS_CHOICES, default="draft", verbose_name="وضعیت")
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)
    
    class Meta:
        verbose_name = "صفحه"
        verbose_name_plural = "صفحات"
    
    def __str__(self):
        return self.title
```

#### Admin Registration

```python
@admin.register(Page)
class PageAdmin(admin.ModelAdmin):
    list_display = ("title", "slug", "status", "updated_at")
    prepopulated_fields = {"slug": ("title",)}
    fields = ("title", "slug", "content", "image", "status")
    list_filter = ("status", "created_at")
    search_fields = ("title",)
```

#### View + URL

```python
def page_view(request, slug):
    page = get_object_or_404(Page, slug=slug, status="published")
    return render(request, "store/page.html", {"page": page})

# urls.py
path("page/<slug:slug>/", views.page_view, name="page"),
```

#### Template (`store/templates/store/page.html`)

A minimal wrapper that extends `base.html`, renders the hero image, title, and `{{ page.content|safe }}` in a `prose prose-violet` container with `max-w-4xl` width.

#### Workflow

1. Admin creates a Page in `/admin/` → fills title, slug, HTML content, optional image, sets "published"
2. Page is live at `/page/<slug>/`
3. Can be linked from anywhere: navigation, hero CTA, footer, blog posts
4. Admin edits content any time through the same admin form — no git push, no server restart

#### Limitations

- Content field is raw HTML — admin needs basic HTML literacy or a WYSIWYG editor
- No block-based layout (for that, upgrade to Wagtail CMS)
- No revision history (Django's `django-simple-history` can be added later)

---

## Two-Tier Auth — Customer Login vs Admin Panel

Create in `store/views.py`:

```python
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.models import User
from django.shortcuts import render, redirect

def user_login(request):
    if request.method == 'POST':
        username = request.POST.get('username')
        password = request.POST.get('password')
        user = authenticate(request, username=username, password=password)
        if user is not None:
            login(request, user)
            return redirect('store:product_list')
        return render(request, 'store/login.html', {'error': 'نام کاربری یا رمز عبور اشتباه است'})
    return render(request, 'store/login.html')

def user_signup(request):
    if request.method == 'POST':
        username = request.POST.get('username')
        password1 = request.POST.get('password1')
        password2 = request.POST.get('password2')
        if password1 != password2:
            return render(request, 'store/signup.html', {'error': 'رمزهای عبور یکسان نیستند'})
        if User.objects.filter(username=username).exists():
            return render(request, 'store/signup.html', {'error': 'این نام کاربری قبلاً ثبت شده است'})
        user = User.objects.create_user(username=username, password=password1)
        login(request, user)
        return redirect('store:product_list')
    return render(request, 'store/signup.html')

def user_logout(request):
    logout(request)
    return redirect('store:product_list')
```

### URLs (`store/urls.py`):

```python
urlpatterns = [
    path("login/", views.user_login, name="user_login"),
    path("signup/", views.user_signup, name="user_signup"),
    path("logout/", views.user_logout, name="user_logout"),
]
```

### Header — Auth-State-Aware Nav

In `base.html`, wrap the login/admin buttons in an `{% if user.is_authenticated %}` check:

```django
<div class="flex items-center gap-4">
  {% if user.is_authenticated %}
  <a href="{% url 'store:user_logout' %}">خروج</a>
  {% else %}
  <a href="{% url 'store:user_login' %}">ورود</a>
  <a href="/admin/">پنل ادمین</a>
  {% endif %}
</div>
```

The admin link is always visible but only `is_staff` users can log in — Django's admin redirects regular users back to login.

### Login/Signup Template Style

Each page is a centered card (`max-w-md`, white bg, `rounded-3xl`, shadow) with:
- Header icon + title
- Form fields (username, password/password2)
- Error banner in `bg-red-50 border-red-200 text-red-700` when auth fails
- Link to the other action (login ↔ signup)
- Purple submit button (`bg-gradient-to-r from-violet-600 to-violet-700`)
- `{% csrf_token %}` in every POST form

## Login Loop Fix

After login, user is redirected back to login page (302 → login again, never reaching dashboard).

**Root cause:** `DEBUG=False` causes Django to set `CSRF_COOKIE_SECURE` and `SESSION_COOKIE_SECURE` to `True` by default. On plain HTTP (no HTTPS), the browser refuses to attach these secure cookies, so every POST fails CSRF validation silently.

**Fix:**
```python
CSRF_COOKIE_SECURE = False
SESSION_COOKIE_SECURE = False
SESSION_COOKIE_HTTPONLY = True
SESSION_EXPIRE_AT_BROWSER_CLOSE = True
```

Also run `python manage.py clearsessions` after changing, and test in Incognito/Private window to bypass cached cookies.

### ⇒ User Preference: Persian + Ultra-Terse Responses + Declarative-Statement Protocol

When the user speaks Persian, **communicate in Persian by default** (casual/friendly tone, NOT formal). Keep responses **ultra-terse**: maximum compression, telegraphic.

- Abbreviate (DB/auth/config/req/res/fn/impl), strip conjunctions, use arrows for causality (X → Y).
- One word when one word enough. Pattern: `[thing] → [result]. [fix].`
- Do NOT write: "Sure! I'd be happy to help with that. The issue you're experiencing is likely caused by…"
- Do write: "Bug in auth middleware. Token expiry check uses `<` not `<=`. Fix:"
- **Table format for mobile Telegram**: Markdown tables render broken on mobile. Use compact inline lists or brief text instead of pipe tables when you know the user is reading on phone.
- **No video/image analysis**: The vision tool only handles static images (jpg/png). If user sends MP4/GIF, tell them to send a screenshot instead.
- **No filler**: After debugging/fixing, confirm the fix worked with a brief status (`✅ fix: 3 things changed`), then stop. Don't re-explain everything.

#### ✅  CRITICAL — Declarative-Statement Protocol:
When the user sends a declarative statement (not ending with `?`), that signals **END of topic** — respond with a brief acknowledgment (1-2 words max) and STOP. Do NOT add anything, continue the topic, ask "چی کار کنیم؟", or propose next steps.

**Declarative signals (STOP after brief ack):**
- `دمت گرم`, `خب اوکی`, `جالب بود دمت‌گرم`, `ببینیم چی میشه`, `مرسی`, `باشه`
- Statements starting with `ببین` followed by a new directive → handle the new topic, then stop
- `خب یه چیزی` introducing a new topic → handle the new request, do NOT loop back to the previous topic
- Any sentence that ends without `?` and is not an explicit instruction

**Continue signals (proceed normally):**
- Sentences ending with `?`
- Explicit requests: `اینو درست کن`, `بگو`, `یاد بگیر`, `ادامه بده`
- `یه سوال` or `چیزی` introducing a question

**Exception:** security warnings, irreversible action confirmations, and multi-step ordered sequences where fragment ambiguity risks misread — write these in full normal prose, then resume terse.

**Boundary clarity check:** If the user repeats the same question or the task genuinely needs more context to proceed, briefly clarify — then immediately resume terse. Do NOT assume every pause means "I need more."

This preference is embedded at the skill level because every deployment session involves explaining/fixing, and this user has repeatedly corrected verbosity + declarative continuation.

<br>

## Cart / Shopping Cart Implementation

A complete cart system with guest+authenticated dual mode, quantity limits, AJAX add button, cart badge in header, and Telegram checkout.

**View the companion references:**
- `references/cart-implementation.md` — full code dump of models, views, URLs, templates, bot handlers
- `references/cart-guest-migration.md` — how guest cart items survive login
- `references/cart-ajax-badge.md` — `addToCart()` JS function + `cart/count/` endpoint
- `references/cart-quantity-500-debugging.md` — why `get_object_or_404(CartItem, ..., cart=cart)` causes 500 and the fix

### Model

```python
class Cart(models.Model):
    user = models.OneToOneField(User, on_delete=models.CASCADE, null=True, blank=True)
    session_key = models.CharField(max_length=255, null=True, blank=True)
    token = models.CharField(max_length=64, unique=True, null=True, blank=True)

    def total_price(self):
        return sum(item.product.price * item.quantity for item in self.items.all())

    def item_count(self):
        return self.items.count()

class CartItem(models.Model):
    cart = models.ForeignKey(Cart, related_name='items', on_delete=models.CASCADE)
    product = models.ForeignKey(Product, on_delete=models.CASCADE)
    quantity = models.PositiveIntegerField(default=1)
```

### Cart Identity (get_or_create_cart)

```python
def get_or_create_cart(request):
    if request.user.is_authenticated:
        cart, created = Cart.objects.get_or_create(user=request.user)
    else:
        if not request.session.session_key:
            request.session.create()
        session_key = request.session.session_key
        if session_key:
            cart, created = Cart.objects.get_or_create(session_key=session_key)
        else:
            cart, created = Cart.objects.get_or_create(id=99999)

    if not cart.token:
        cart.token = secrets.token_urlsafe(32)
        cart.save()
    return cart
```

⚠️ **CRITICAL GOTCHA**: Cart with `user=None` AND `session_key=None` causes `get_or_create` to create a **new cart every time** → 500 errors on cart page. Always ensure session_key is non-null for guest carts.

### Quantity Limits

```python
def add_to_cart(request, code):
    cart = get_or_create_cart(request)
    product = get_object_or_404(Product, code=code)
    item, created = CartItem.objects.get_or_create(cart=cart, product=product)
    if not created:
        max_qty = product.stock
        if product.price > 2000000:
            max_qty = min(max_qty, 1)
        if item.quantity >= max_qty:
            return redirect('store:cart_view')
        item.quantity += 1
        item.save()
    else:
        if product.price > 2000000:
            item.quantity = 1
        elif item.quantity > product.stock:
            item.quantity = product.stock
        item.save()
    return redirect('store:cart_view')
```

Rules: stock cap (qty ≤ stock) AND price cap (product > 2M Toman = max 1).

### Quantity +/− Controls with Cart Migration

- **Template `{% url %}` name mismatch → silent 500 on page render** — Using `{% url 'store:cart_add_one' item.product.code %}` when the URL is named `cart_add` (not `cart_add_one`) causes Django's `NoReverseMatch` at template render time → 500 with no visible traceback. **Always verify URL names match between `urls.py` and `templates/` after renaming.** Quick check: `grep -n "store:" templates/store/cart.html | grep -E "url 'store:(cart|checkout)"` and cross-check against `urls.py`.

- **Template `{% url %}` name mismatch → silent 500 on page render** — Using `{% url 'store:cart_add_one' item.product.code %}` when the URL is named `cart_add` (not `cart_add_one`) causes Django's `NoReverseMatch` at template render time → 500 with no traceback in the browser. **Always verify URL names match between `urls.py` and `templates/` after renaming.** Quick check: `grep -rn "template_url_name" templates/ | grep -v "expected_name"`.

```python
def update_cart_quantity(request, item_id, delta):
    cart = get_or_create_cart(request)
    try:
        item = CartItem.objects.get(id=item_id)
    except CartItem.DoesNotExist:
        return redirect('store:cart_view')

    # If item belongs to a different cart (session changed), migrate it
    if item.cart != cart:
        existing = CartItem.objects.filter(cart=cart, product=item.product).first()
        if existing:
            existing.quantity += item.quantity
            existing.save()
            item.delete()
            item = existing
        else:
            item.cart = cart
            item.save()

    # Apply stock + price caps
    new_qty = item.quantity + delta
    if new_qty <= 0:
        item.delete()
        return redirect('store:cart_view')

    max_qty = item.product.stock
    if item.product.price > 2000000:
        max_qty = 1
    if new_qty > max_qty:
        return redirect('store:cart_view')

    item.quantity = new_qty
    item.save()
    return redirect('store:cart_view')
```

**WARNING**: `get_object_or_404(CartItem, id=item_id, cart=cart)` causes 500 when the cart ID mismatches (session drift). Always use the try/except + migration pattern above.

### Primary Image in Cart Template

**WRONG** (template chaining silently fails):
```django
<img src="{{ item.product.images.filter(is_primary=True).first.image.url }}">
```

**RIGHT** (pre-compute in view):
```python
def cart_view(request):
    cart = get_or_create_cart(request)
    for item in cart.items.all():
        item.primary_image = item.product.images.filter(is_primary=True).first()
    return render(request, 'store/cart.html', {'cart': cart})
```

Then in template:
```django
<img src="{{ item.primary_image.image.url }}">
```

### Cart Checkout → Telegram Deep Link

```python
def checkout_to_telegram(request):
    cart = get_or_create_cart(request)
    if not cart.items.exists():
        return redirect('store:cart_view')
    return redirect(f"https://t.me/{BOT_USERNAME}?start=cart_{cart.token}")
```

### Bot Handler for start=cart_<token>

```python
if args and args[0].startswith("cart_"):
    token = args[0][5:]
    cart = await sync_to_async(Cart.objects.filter(token=token).prefetch_related("items__product__type").first)()
    if not cart or not cart.items.exists():
        await update.message.reply_text("❌ سبد خرید یافت نشد.")
        return

    msg = "🛒 *سبد خرید شما*\n\n"
    total = 0
    for idx, item in enumerate(cart.items.all(), 1):
        p = item.product
        msg += f"{idx}. *{p.name}* — {p.price:,} تومان\n"
        total += p.price * item.quantity
    msg += f"\n💰 *جمع کل:* {total:,} تومان"

    await update.message.reply_text(msg, parse_mode="Markdown",
        reply_markup=InlineKeyboardMarkup([[
            InlineKeyboardButton("✅ تأیید کل سبد", callback_data=f"cart_confirm:{token}"),
            InlineKeyboardButton("❌ انصراف", callback_data="order_cancel"),
        ]]))
```

Register:
```python
app.add_handler(CallbackQueryHandler(cart_confirm_handler, pattern="^cart_confirm:"))
```

### Cart Confirm Handler (bulk approval)

```python
async def cart_confirm_handler(update, context):
    query = update.callback_query
    if not query: return
    await query.answer()
    token = query.data.split(":")[1]
    cart = await sync_to_async(Cart.objects.filter(token=token).prefetch_related("items__product__type").first)()
    for item in cart.items.all():
        await sync_to_async(PromotionQueueItem.objects.create)(product=item.product)
    # Notify admin with cart summary
    ...
```

### Admin Notification — Use Numeric chat_id ONLY

```python
# CRITICAL: chat_id="@username" is silently dropped if bot hasn't received /start from that user
# CRITICAL: chat_id="123456" (numeric) ALWAYS works once bot has any conversation with that user
await context.bot.send_message(chat_id=95851963, text=msg, parse_mode="HTML")
```

Never use `@username` for admin notifications.

### Verification Quick-Check

```bash
# Does cart page render without 500?
curl -s -o /dev/null -w "%{http_code}" http://localhost:8000/cart/
# Is the cart page HTML valid (check for unclosed tags)?
curl -s http://localhost:8000/cart/ | grep -c "</p>"  # should match <p> count
# Does image src url exist?
curl -s http://localhost:8000/cart/ | grep -o 'src="[^"]*"' | head -3
# Check images actually 200
curl -s -o /dev/null -w "%{http_code}" http://localhost:8000/media/products/somefile.jpg
```

## MEDIA_ROOT Must Match Nginx Alias

**Problem:** Django writes uploads to `MEDIA_ROOT`. Nginx reads from its `location /media/ alias`. If these paths differ, uploads work but images 404 on the storefront. Nginx runs as `www-data`; `/root/` has `700` permissions → **403 Forbidden** on every image.

**Fix — point both to `/var/www/`:**

```python
# settings.py
MEDIA_ROOT = "/var/www/resin-media"
```

```nginx
# nginx conf
location /media/ { alias /var/www/resin-media/; }
```

**Initial sync:**
```bash
cp -r /project/website/media /var/www/resin-media
chown -R www-data:www-data /var/www/resin-media
```

**Ongoing sync** (after changing MEDIA_ROOT, old files stay at old path):
```bash
rsync -av --delete /old/media/path/ /var/www/resin-media/
```

## TinyMCE WYSIWYG Editor in Django Admin (CDN — No pip install)

### 1. Override the admin template

Create `templates/admin/store/page/change_form.html`:

```django
{% extends "admin/change_form.html" %}
{% load static %}

{% block admin_change_form_document_ready %}
{{ block.super }}
<script>
(function() {
  if (typeof tinymce === 'undefined') return;

  const contentField = document.querySelector('#id_content');
  if (!contentField) return;

  tinymce.init({
    target: contentField,
    height: 500,
    menubar: true,
    plugins: 'lists link image table code help',
    toolbar: 'undo redo | bold italic underline strikethrough | fontselect fontsizeselect | alignleft aligncenter alignright alignjustify | outdent indent |  numlist bullist | image link | removeformat | code help',
    directionality: 'rtl',
    language: 'fa',
    promotion: false,
    setup: function (editor) {
      editor.on('init', function () {
        editor.getContainer().style.direction = 'rtl';
      });
    },
  });
})();
</script>
{% endblock %}
```

### 2. Register Media + context in admin.py

```python
@admin.register(Page)
class PageAdmin(admin.ModelAdmin):
    # ... existing config ...

    class Media:
        js = ('https://cdn.jsdelivr.net/npm/tinymce@6/tinymce.min.js',)

    def changeform_view(self, request, object_id=None, form_url='', extra_context=None):
        extra_context = extra_context or {}
        extra_context['use_tinymce'] = True
        return super().changeform_view(request, object_id, form_url, extra_context)
```

### 3. Register the template directory in settings.py

```python
# website/settings.py — TEMPLATES list
'DIRS': [BASE_DIR / 'templates'],
```

---

## RTL Frontend + LTR Admin Layout

For a Persian shop that needs full RTL on the storefront but LTR layout in the Unfold admin panel (sidebar stays left, text is right-aligned), see `references/rtl-site-ltr-admin-unfold.md` for the CSS override approach.

⚠️ **Do NOT use `UNFOLD["STYLES"]`** — causes HTTP 500 on all admin pages. Use `EXTRA_STYLES` (inline CSS strings) or `EXTRA_CSS` (stylesheet file URL). If you already added `STYLES`, remove the key and restart gunicorn.

## Product Auto-Code Generation — Filter by Type

**Bad — range filter that crosses type boundaries:**
```python
max_code = Product.objects.filter(
    code__gte=start, code__lt=start + 10000
).aggregate(m=Max("code"))["m"]
```

**Fix — filter by product type:**
```python
max_code = Product.objects.filter(type=self.type).aggregate(m=Max("code"))["m"]
self.code = start if max_code is None else max_code + 1
```

## Product Filter + Sort Dropdowns (Advanced)

Use dropdown menus (not inline pills) for categories + separate sort dropdown. See `references/product-search-filter-pattern.md` for full code.

### Key rules:
- **Home page (/)**: Header → Hero → 3 featured product cards → Footer. NEVER redirect home to products.
- **Products page (/products/)**: Search bar + category dropdown + sort dropdown + product grid + CTA.
- **Sort options**: newest (default), price_asc, price_desc, stock.
- **Dropdown links must preserve ALL current params** (`q`, `type`, `sort`).
- **CTA `<p>` tag**: Always close with `</p>` — the closing tag is frequently eaten by template markup, causing layout breakage.
- **Logo check**: `curl -I http://domain.com/media/logo.jpg` (via nginx = 200) NOT `localhost:8000` (via gunicorn = 404 expected in DEBUG=False).

```python
# View with sort
def product_list(request):
    type_name = request.GET.get("type")
    search_query = request.GET.get("q", "").strip()
    sort_by = request.GET.get("sort", "newest")
    
    qs = Product.objects.select_related("type")
    
    if type_name and type_name != "all":
        qs = qs.filter(type__name=type_name)
    if search_query:
        qs = qs.filter(
            models.Q(name__icontains=search_query) | 
            models.Q(code__icontains=search_query) |
            models.Q(type__name__icontains=search_query)
        )
    
    if sort_by == "price_asc":      qs = qs.order_by("price")
    elif sort_by == "price_desc":   qs = qs.order_by("-price")
    elif sort_by == "stock":        qs = qs.order_by("-stock", "-id")
    else:                           qs = qs.order_by("-id")  # newest
    
    return render(request, "store/product_list.html", {... "current_sort": sort_by})
```

### Template: dropdown + sort (see reference for full HTML)

### Home view (NEVER redirect to /products/)

```python
def home(request):
    featured = Product.objects.select_related("type").order_by("-id")[:3]
    for p in featured:
        p.primary_image = p.images.filter(is_primary=True).first()
    return render(request, "store/home.html", {"featured_products": featured})
```

## Docker Compose Setup

Create `docker-compose.yml` at the project root:

```yaml
services:
  db:
    image: postgres:16-alpine
    environment:
      POSTGRES_DB: ${DB_NAME:-shop}
      POSTGRES_USER: ${DB_USER:-shop}
      POSTGRES_PASSWORD: ${DB_PASSWORD:?required}
    volumes:
      - pgdata:/var/lib/postgresql/data
    restart: unless-stopped

  redis:
    image: redis:7-alpine
    restart: unless-stopped

  web:
    build: .
    command: >
      sh -c "python manage.py migrate &&
             python manage.py collectstatic --noinput &&
             gunicorn website.wsgi:application --bind 0.0.0.0:8000 --workers 4"
    environment:
      DJANGO_SECRET_KEY: ${DJANGO_SECRET_KEY:?required}
      DJANGO_DEBUG: ${DJANGO_DEBUG:-False}
      DB_HOST: db
      DB_NAME: ${DB_NAME:-shop}
      DB_USER: ${DB_USER:-shop}
      DB_PASSWORD: ${DB_PASSWORD:?required}
      TELEGRAM_BOT_TOKEN: ${TELEGRAM_BOT_TOKEN:?required}
      TELEGRAM_CHANNEL_CHAT_ID: ${TELEGRAM_CHANNEL_CHAT_ID:?required}
      TELEGRAM_LOG_CHAT_ID: ${TELEGRAM_LOG_CHAT_ID:-}
      CELERY_BROKER_URL: redis://redis:6379/0
      CELERY_RESULT_BACKEND: redis://redis:6379/1
    volumes:
      - static_volume:/app/static
      - media_volume:/app/media
    depends_on:
      - db
      - redis
    restart: unless-stopped

  nginx:
    image: nginx:alpine
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
      - static_volume:/static:ro
      - media_volume:/media:ro
    ports:
      - "80:80"
    depends_on:
      - web
    restart: unless-stopped

  bot:
    build: .
    command: python -m store.bot
    environment:
      DJANGO_SETTINGS_MODULE: website.settings
      DB_HOST: db
      DB_NAME: ${DB_NAME:-shop}
      DB_USER: ${DB_USER:-shop}
      DB_PASSWORD: ${DB_PASSWORD:?required}
      TELEGRAM_BOT_TOKEN: ${TELEGRAM_BOT_TOKEN:?required}
      TELEGRAM_CHANNEL_CHAT_ID: ${TELEGRAM_CHANNEL_CHAT_ID:?required}
      CELERY_BROKER_URL: redis://redis:6379/0
    depends_on:
      - db
      - redis
    restart: unless-stopped

  celery_worker:
    build: .
    command: celery -A website worker -l info
    environment: &celery_env
      DJANGO_SETTINGS_MODULE: website.settings
      DB_HOST: db
      DB_NAME: ${DB_NAME:-shop}
      DB_USER: ${DB_USER:-shop}
      DB_PASSWORD: ${DB_PASSWORD:?required}
      CELERY_BROKER_URL: redis://redis:6379/0
      CELERY_RESULT_BACKEND: redis://redis:6379/1
    depends_on:
      - db
      - redis
    restart: unless-stopped

  celery_beat:
    build: .
    command: celery -A website beat -l info
    environment: *celery_env
    depends_on:
      - db
      - redis
      - celery_worker
    restart: unless-stopped

volumes:
  pgdata:
  static_volume:
  media_volume:
```

### Nginx Config (`nginx.conf`)

```nginx
events {
    worker_connections 1024;
}

http {
    include /etc/nginx/mime.types;
    upstream django {
        server web:8000;
    }

    server {
        listen 80;
        server_name _;

        location /static/ { alias /static/; }
        location /media/ { alias /media/; }

        location / {
            proxy_pass http://django;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        }
    }
}
```

## Running Celery Tasks Without Redis (Low-RAM Alternative)

If RAM < 1GB, replace Redis + Celery with a cron job:

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

Then `chmod +x /opt/run-promotion.sh` and add to crontab: `0 10 * * * /opt/run-promotion.sh`

## Celery 5.x Compatibility — No `--daemon` Flag

**⚠️ CRITICAL:** Celery 5.x removed the `--daemon` flag. The project's `run.sh` uses:
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
Also update `run.sh` — remove `--daemon` flags everywhere.

## PromotionQueueItem Bug — Orders Polluting the Channel Queue

**Bug:** `views.py:send_order_to_bot()` creates a `PromotionQueueItem` for every order:
```python
PromotionQueueItem.objects.create(product=product)  # line 194
```
This pollutes the queue meant **only for channel promotions**. The admin list shows all items (PENDING + POSTED + FAILED from orders), and `process_promotion_queue` skips them (wrong status).

**Fix — change `send_order_to_bot` to NOT create PromotionQueueItem:**\n```python\n# In send_order_to_bot (views.py:190-215)\n# REMOVE line 194: PromotionQueueItem.objects.create(product=product)\n# Keep only the admin notification logic (lines 196-215)\n```\nThe admin notification already works — it sends directly to Telegram, no queue needed.\n\n**Admin — use `list_filter`, NOT `get_queryset` override:**\nUser explicitly rejects hard `get_queryset` filter. Just use `list_filter`:\n```python\nclass PromotionQueueItemAdmin(ModelAdmin):\n    list_display = (\"id\", \"product\", \"status\", \"attempts\", \"created_at\", \"locked_at\", \"posted_at\")\n    list_filter = (\"status\", \"created_at\")  # ← user picks from dropdown\n    search_fields = (\"product__name\", \"product__code\")\n    autocomplete_fields = (\"product\",)\n    ordering = (\"-created_at\",)\n    # NO get_queryset() override — user will use list_filter manually\n```\n\n**GitHub push rejected by 100MB file size limit:**\nWhen pushing `.hermes/` to GitHub for full server migration, two large binaries exceed the limit:\n- `.hermes/node/bin/node` (119 MB)\n- `.hermes/bin/uv` (61 MB)\n**Fix:** add to `.gitignore`:\n```\n.hermes/node/\n.hermes/bin/uv\n.hermes/bin/uvx\n```\nThese are re-downloaded by `hermes setup` on the new server — not needed in git.\n\n**User directive — include ALL data in git push (no filtering):**\nWhen the user says \"push everything to GitHub\", do NOT filter or decide what's \"safe\". Include `.hermes/`, `db.sqlite3`, `.env`, everything needed to clone and run on a new server. Only exclude files that physically cannot fit (see 100MB limit above). User overrides security concerns.\n\n**Pitfalls & Gotchas**

1. **Bot and Django share DB but not process**: Bot makes its own `django.setup()` call. Ensure both can reach the database with the same credentials.
2. **Celery Beat schedule timezone**: Set `CELERY_TIMEZONE = "Asia/Tehran"` and `CELERY_ENABLE_UTC = False` for local-time scheduling.
3. **Unfold admin theme**: Requires `django-unfold` in requirements, and `'unfold'` before `'django.contrib.admin'` in `INSTALLED_APPS`.
4. **Bot state in memory**: python-telegram-bot stores user_data in memory by default. For production, use `Persistence` (e.g. `PicklePersistence`) to survive restarts.
5. **Async ↔ Sync bridge**: Bot handlers use `sync_to_async` wrappers for Django ORM calls. Ensure all DB queries go through these wrappers, not directly in async context.
6. **Media files in Docker**: Use Docker volumes, not SQLite. SQLite in a container is fragile and slow. PostgreSQL is strongly recommended.
7. **Static file serving**: Gunicorn does NOT serve static files. Always use Nginx or a CDN for static/media in production.
8. **`.gitignore` checklist**: Add `*.pyc`, `__pycache__/`, `.env*`, `db.sqlite3`, `celerybeat-schedule`, `media/`, `static/`.
9. **Template gotcha — no method chaining**: Django templates cannot call `.filter(...).first()`. Pre-compute in the view (e.g. `p.primary_image = p.images.filter(is_primary=True).first()`) then use `p.primary_image` in template.
10. **OOM on low-RAM VPS**: Killed process exits with 137. Diagnose with `dmesg | grep -i oom`. Immediate fix: `systemctl stop redis` + `pkill -f celery` + reduce gunicorn workers to 1. Long-term: cron-based celery replacement (see section above). Also run `free -h` + `swapon --show` to assess swap usage.
11. **UNFOLD config invalid key**: `STYLES` is not a valid Unfold key — causes HTTP 500 with no traceback in Django. Do NOT use. Instead use `EXTRA_STYLES` (list of CSS inline strings) or `EXTRA_CSS` (list of CSS file URLs). If you already added `STYLES`, remove the key and restart gunicorn.
12. **Nginx /root/ permissions**: Never point Nginx alias to `/root/`. Copy static/media to `/var/www/` and `chown -R www-data:www-data`.
13. **LANGUAGE_CODE = 'fa' forces admin RTL**: Setting `LANGUAGE_CODE = 'fa'` makes Unfold admin fully RTL (sidebar on right). To keep admin LTR while site is RTL: use `LANGUAGE_CODE = 'en-us'` in settings, keep `dir="rtl"` only in the site's `base.html`. Persian content renders correctly in LTR mode — only the layout changes.
14. **Port conflict on restart**: `gunicorn` or `bot.py` may fail with `Address already in use` when a previous process still holds the port. Free with `fuser -k 8000/tcp` (for gunicorn) or `fuser -k PORT/tcp`. Verify with `ss -tlnp | grep PORT`.
15. **Gunicorn supervisor**: `nohup gunicorn &` is fragile — process dies on SSH logout / OOM. Use systemd service (see `references/server-migration-low-ram.md`) for auto-restart. Set `Restart=always`.
- **`telegram.error.Conflict` multi-bot instance** — Only ONE bot instance can run per Telegram bot token. Before starting a new bot on a new server, kill any running bot instance on the old server (`pkill -f bot.py`).
- **Nginx duplicate server block** — Editing `/etc/nginx/sites-available/resin` multiple times can append a duplicate `server {}` block. Reload then fails with cryptic errors or the first block serves all traffic. After every edit, read the file and check for duplicates before reloading.
- **ESP32-S3 Servo Pitfalls**: See `references/esp32-s3-servo-pitfalls.md` for GPIO restrictions, PWM conflicts, and new LEDC API usage.
- **Bot dies on Telegram API timeout**: Iran ISPs sometimes throttle api.telegram.org, causing `TimedOut` error. Bot exits with no visible error. Wrap in systemd service with `Restart=always` so it auto-recovers.
- **Logo 404 on localhost:8000 but 200 on domain**: This is EXPECTED when `DEBUG=False`. Gunicorn does NOT serve media files in production mode. Nginx handles `/media/` paths. Always test media via the domain URL, not localhost port. `curl http://localhost:8000/media/logo.jpg` → 404 (gunicorn), `curl http://domain.com/media/logo.jpg` → 200 (nginx). Not a bug.
19. **TinyMCE init race**: Wrap in `$(document).ready()` or `setTimeout(..., 0)`.
19. **Import destruction**: Adding `from django.urls import reverse` without keeping `from django.shortcuts import render, redirect, get_object_or_404` breaks ALL views. Always APPEND, never replace.


## Verification Pitfalls (CRITICAL — Embedded from Session)

**NEVER say "done" or "applied" without verifying the result is actually visible to the user.**

### 1. Nginx media files return 404
**Symptom:** File exists in `/var/www/resin-media/` but `curl localhost:8000/media/file` returns 404.
**Root cause:** Nginx config `alias` path must EXACTLY match the directory where files are copied.
**Fix:**
```bash
# Verify file exists
ls -la /var/www/resin-media/logo.jpg
# Test via localhost (bypasses DNS)
curl -s -o /dev/null -w "%{http_code}" http://localhost:8000/media/logo.jpg
# If 404, check nginx config
cat /etc/nginx/sites-available/resin
# Reload nginx after fixing
systemctl reload nginx
```

### 1. CTA `<p>` tag layout breakage

**Symptom:** The CTA section text collapses into buttons (text appears on same line as buttons instead of above them).

**Root cause:** The `<p>` tag in the CTA section lost its closing `</p>` — template markup with Persian content (`.` + `</` sequence) gets corrupted during Django template rendering when the `</p>` immediately follows a Cyrillic-origin punctuation like `.` with a space and block element.

**Fix:** Always verify every `<p>` tag in a template has a proper closing `</p>` on its own line:
```django
<!-- BAD: layout breaks -->
<p class="...">متن فارسی با نقطه.</    <div class="...">

<!-- GOOD: layout works -->
<p class="...">متن فارسی با نقطه.</p>
<div class="...">
```

**Pattern:** After editing any template with Persian text + CTA sections, `grep -c "<p"` vs `grep -c "</p>"` — if counts don't match, fix immediately. Also grep for `.</ ` (dot + tag-start) sequences which indicate the corruption.

### 2. CSS/template changes not visible
**Symptom:** User says "nothing changed" after CSS or template updates.
**Cause:** Browser cache or Nginx caching old files.
**Fix:**
```bash
# Verify the change is in the served HTML
curl -s http://localhost:8000/ | grep "new-class-or-text"
# If grep finds it but user doesn't see it → browser cache → tell user to Hard Refresh (Ctrl+F5)
```

### 3. Persian-Content Specific: `<p>` tag must be on its own line
In Persian content templates (base.html, product_list.html, home.html), the sequence `نقطه.</    <div` (closing `</p>` on same line as a block element) gets corrupted by Django template engine. **Always put `</p>` on its own line**, never followed by a block element on the same line.

### 4. Saying "done" without curl test
**Pattern:** Apply changes → restart gunicorn → say "done" → user opens page → nothing changed.
**Correct pattern:** Apply changes → restart gunicorn → `curl -s http://localhost:8000/ | grep "expected output"` → confirm match → THEN say "done".

### 4. Nginx duplicate server block
**Symptom:** Editing `/etc/nginx/sites-available/resin` multiple times appends a duplicate `server {}` block. Reload then fails or serves wrong config.
**Fix:** After every edit, `cat /etc/nginx/sites-available/resin` and check for duplicates before reloading.

### 5. MEDIA_URL context processor missing
**Symptom:** `{{ MEDIA_URL }}` renders empty in templates → logo `src="/media/logo.jpg"` instead of `src="/media/logo.jpg"` (missing leading slash).
**Root cause:** `django.template.context_processors.media` missing from `TEMPLATES[0].OPTIONS.context_processors`.
**Fix:** Add `'django.template.context_processors.media',` to `context_processors` list in `settings.py` → restart gunicorn.

### 5. TinyMCE init race condition
**Symptom:** TinyMCE doesn't initialize on admin page — textarea remains plain.
**Cause:** TinyMCE init script runs before DOM ready when placed in `block admin_change_form_document_ready` without wrapper.
**Fix:** Wrap init in `$(document).ready()` or use `setTimeout(..., 0)`:
```javascript
$(document).ready(function() {
  tinymce.init({ ... });
});
// OR
setTimeout(function() { tinymce.init({ ... }); }, 0);
```

### 6. Nginx config test before reload

## Verification

```bash
# All containers healthy?
docker compose ps
```
# Django reachable?
curl -s http://localhost/admin/ | head -5

# Bot started?
docker compose logs bot | grep -i "started\|polling\|error"

# Celery working?
docker compose logs celery_worker | tail -5

# Can send to channel?
docker compose exec bot python -c "
import django; django.setup()
from django.conf import settings
print('Token exists:', bool(settings.TELEGRAM_BOT_TOKEN))
print('Channel ID:', settings.TELEGRAM_CHANNEL_CHAT_ID)
"
```

## Brand Customization — Logo + Color Palette from Image

### 🧾 CRITICAL: Personal content never on public paths
When the user asks for personal/off-topic content (posters, character collages, private files), NEVER save/upload to public site paths (media/, templates/ that are under nginx). Serve via local-only means (local file path, localhost URL). User has explicitly enforced this rule.

### Palette Rule for this user: minimalist, neutral, soft
- Avoid aggressive purples with high red content (#6B2D7B rejected as too vibrant)
- Use muted purples like #6D5B95, soft pinks #E5B3B4, sage greens #8FA998
- Persian text ALWAYS preferred over English in site UI (header, footer, brand captions)
- Test hard refresh (Ctrl+Shift+R) after every palette/logo change — nginx caching is aggressive

### Upload workflow (no git, no SSH editor needed)

1. User sends logo image in chat → save to `/root/resin-web/website/media/logo.jpg`
2. Copy to Nginx-served dir: `cp /root/resin-web/website/media/logo.jpg /var/www/resin-media/logo.jpg && chown www-data:www-data /var/www/resin-media/logo.jpg`
3. In `base.html`, replace the placeholder logo block:
   - Swap `<div>م</div>` gradient circle with `<img src="{{ MEDIA_URL }}logo.jpg" alt="Brand" class="h-12 w-auto rounded-lg">`
   - Swap hardcoded brand text (e.g. `"مه‌گل"`) with the logo image + brand caption
4. In the footer, add the logo the same way: `<img src="{{ MEDIA_URL }}logo.jpg" alt="" class="h-10 w-auto rounded-lg">`
5. Update shadow/hero gradients to match the logo's palette:
   - Extract dominant colors from the logo visually (or ask the user for hex values)
   - Replace `violet-*` throughout templates with the new palette
   - Note: `rgba()` values used in CSS custom properties (like `rgba(124,58,237,0.12)`) MUST also be updated — they don't change automatically when Tailwind classes change
6. Test: hard refresh (Ctrl+Shift+R) on domain — Nginx caches the image aggressively

### Palette extraction from an image (manual)

When the user provides a logo but no hex codes:
- Describe the dominant colors you see in the image (foreground, background, accent)
- Propose a Tailwind-compatible palette (primary-600/800, accent-400/500, neutral)
- Let the user confirm before applying changes across templates
- Apply changes via `sed` for bulk replacement: `sed -i 's/violet-600/purple-700/g'`

### Pitfalls

- **Logo 404**: Nginx alias directory ≠ Django MEDIA_ROOT. See MEDIA_ROOT section above.
- **Nginx serves old image**: Cache issue. Run `systemctl reload nginx` after copying. User should hard-refresh browser.
- **Only one palette applied**: `h-12` works but `text-violet-200` in footer didn't change because `sed` missed it. Do a full grep of the template directory before claiming the palette is fully ported.

## Handmade-Style Tailwind Templates (RTL/Persian)

For e-commerce shops targeting Iranian users, a clean, minimal, Shopify-inspired template set:
- `templates/store/base.html` — Sticky header with nav blur, hero section, CTA banner, footer
- `templates/store/product_list.html` — Product grid with category filters, hover animations, stock badges
- `templates/store/product_detail.html` — Image gallery grid, sticky sidebar, price card, order CTA
- `templates/store/login.html` — Customer login card (minimal, centered, purple theme)
- `templates/store/signup.html` — Customer signup card (same style, with password validation)

**Key design patterns:**
- Color palette: `violet-600/800` as primary, `amber-400/500` as accent
- Font: `Vazirmatn` loaded from Google Fonts
- RTL: `dir="rtl"` + `lang="fa"` on `<html>`, Tailwind CDN (no build step)
- Responsive: `auto-fill` grid, mobile menu with JS toggle

To adapt to a new brand: replace the violet palette in `base.html` (search `violet-600`) and swap the logo/text.

---

## Start the Stack (Bare-Metal / Non-Systemd Mode)

Project ships `run.sh` at `/root/resin-web/run.sh`. The committed version is **broken** — it uses Celery 4.x `--daemon` flag (removed in Celery 5.x), so celery silently fails to launch. Also the script often lacks execute permission.

**Correct startup** (each component backgrounded explicitly with `nohup`):

```bash
cd /root/resin-web
chmod +x run.sh            # required: script ships non-executable
./run.sh                   # or run the commands below directly

# Gunicorn (Django) — port 8000
cd /root/resin-web/website
source ../venv/bin/activate
nohup gunicorn website.wsgi:application \
  --bind 0.0.0.0:8000 --workers 1 --timeout 120 \
  --access-logfile - > /tmp/gunicorn.log 2>&1 &

# Celery worker + beat (NO --daemon in 5.x)
nohup celery -A website worker -l INFO --concurrency 2 \
  > /tmp/celery-worker.log 2>&1 &
nohup celery -A website beat -l WARNING \
  > /tmp/celery-beat.log 2>&1 &

# Telegram bot
nohup python store/bot.py > /tmp/bot.log 2>&1 &
echo $! > /tmp/bot.pid
```

**Stop:** `kill $(cat /tmp/bot.pid); pkill gunicorn; pkill -f celery`

**Restart after edits:** re-run the blocks above. Django caches `.env` at startup — restart gunicorn AND bot after `.env` changes.

> Low-RAM note: `--workers 1` on 1GB VPS. `--workers 2` in `run.sh` risks OOM. See `references/server-migration-low-ram.md`.

---

## Is the Site "Off"? Diagnose Upstream, Not Nginx

When the user says "nginx is off" / site down, **check the upstream first**:

1. `systemctl status nginx` — nginx is usually `active (running)`; it proxies to `127.0.0.1:8000`.
2. `ss -tlnp | grep -E ':80|:8000'` — if `:80` listens (nginx) but `:8000` does NOT → gunicorn/Django is dead, that's why the site is unreachable.
3. `pgrep -af gunicorn` / `pgrep -af celery` / `pgrep -af "python.*bot.py"` — confirm processes alive.
4. `curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8000/` → expect `200`. `curl http://domain.com/` → expect `200`.

**Fix:** start the stack (above). Nginx rarely needs restarting; the real fix is bringing the dead upstream back.

---

## Verification — Runtime Proof Required (Not Just Static)

After editing a startup script, config, or `settings.py`, static checks (`py_compile`, import resolution) are necessary but NOT sufficient. Always confirm at runtime:

```bash
pgrep -af gunicorn                       # process exists
ss -tlnp | grep :8000                    # port listening
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8000/   # 200
curl -s -o /dev/null -w "%{http_code}" http://u2ssqxd25095.dxdx5.com/  # 200
```

**Media caveat:** `curl localhost:8000/media/logo.jpg` → 404 expected when `DEBUG=False` (gunicorn doesn't serve media). `curl domain.com/media/logo.jpg` → 200 via nginx. Mismatch = nginx alias ≠ MEDIA_ROOT.

> If a service restart is blocked by the user (e.g. they deny `hermes gateway restart`), state clearly that the fix is applied+static-verified but NOT yet live-proven, and give them the exact restart command. Don't claim it's working.

---

## Interop — Hermes Telegram Gateway Bug (Installed Instance)

For a non-root Hermes install, `hermes gateway status` showed:
`ERROR ... NameError: name 'InlineQueryHandler' is not defined` → bot never connects (silent).

**Root cause:** the Telegram adapter's top-level import block (in
`/usr/local/lib/hermes-agent/plugins/platforms/telegram/adapter.py`)
omitted `InlineQueryHandler`/`InlineQueryResultArticle`/`InputTextMessageContent`,
while `connect()` uses them at line ~2922. They were only imported inside a lazy-dependency helper, which isn't always called.

**Fix:** add to the top-level `try: from telegram...` block:
```python
from telegram import InlineQueryResultArticle, InputTextMessageContent
from telegram.ext import (..., InlineQueryHandler)
```
plus the `Any`-stubs for the ImportError branch.

**Verify (no restart needed):**
```bash
/usr/local/lib/hermes-agent/venv/bin/python -m py_compile <adapter.py>   # COMPILE OK
/usr/local/lib/hermes-agent/venv/bin/python -c "from telegram.ext import InlineQueryHandler"  # OK
```
**Runtime proof** needs `hermes gateway restart` (blocked by user this session — left to them). See `references/hermes-telegram-gateway-fix.md`.

> NOTE: edit is in the installed package tree at `/usr/local/lib/hermes-agent/` — `hermes update` overwrites it. Report upstream.
