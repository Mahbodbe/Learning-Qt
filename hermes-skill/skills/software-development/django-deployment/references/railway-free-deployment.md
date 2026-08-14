# Railway Free Deployment (No Credit Card)

## Platform Comparison
- **Railway** ✅ رایگان، کارت نمی‌خواد، GitHub deploy مستقیم
- **Render** ❌ کارت می‌خواد برای free tier
- **Vercel / Netlify** ❌ فقط static — Django نمی‌شه

## nixpacks.toml (Python + Node همزمان)
```toml
[phases.setup]
nixPkgs = ["python312", "nodejs_22"]  # nodejs_22 ضروری — vite 8 / rolldown نیاز به Node >= 20.19 یا >= 22 داره

[phases.build]
cmds = [
  "pip install -r requirements.txt",
  "cd admin-frontend && npm install && npm run build && cd ..",
  "cp -r admin-frontend/dist website/staticfiles/dashboard",
  "python3 website/manage.py collectstatic --noinput"  # python3 نه python
]

[start]
cmd = "cd website && gunicorn website.wsgi:application --bind 0.0.0.0:$PORT --workers 2"
```

## Environment Variables روی Railway
```
SECRET_KEY=...
ALLOWED_HOSTS=*
DEBUG=False
DJANGO_SETTINGS_MODULE=website.settings
CSRF_TRUSTED_ORIGINS=https://your-app.up.railway.app
HTTPS=true
```

## Settings داینامیک (ضروری)
```python
ALLOWED_HOSTS = os.environ.get('ALLOWED_HOSTS', '*').split(',')
CSRF_TRUSTED_ORIGINS = [x for x in os.environ.get(
    'CSRF_TRUSTED_ORIGINS', 'https://*.railway.app,https://*.up.railway.app'
).split(',') if x]
MEDIA_ROOT = os.environ.get('MEDIA_ROOT', BASE_DIR / 'media')
CELERY_BROKER_URL = os.environ.get("REDIS_URL", "redis://127.0.0.1:6379/0")
CELERY_RESULT_BACKEND = os.environ.get("REDIS_URL", "redis://127.0.0.1:6379/1")

# Cookie security — dynamic (False on HTTP, True on HTTPS)
SESSION_COOKIE_SECURE = os.environ.get('HTTPS', 'false').lower() == 'true'
CSRF_COOKIE_SECURE = os.environ.get('HTTPS', 'false').lower() == 'true'
SESSION_COOKIE_SAMESITE = 'Lax'
CSRF_COOKIE_SAMESITE = 'Lax'
```

## Dashboard React بدون Nginx
Railway Nginx نداره — Django باید `/dashboard/` رو serve کنه:

1. `staticfiles/` رو به TEMPLATES DIRS اضافه کن:
```python
'DIRS': [BASE_DIR / 'templates', BASE_DIR / 'staticfiles'],
```

2. توی `urls.py`:
```python
from django.views.generic import TemplateView
from django.views.static import serve
from django.urls import re_path

path('dashboard/', TemplateView.as_view(template_name='dashboard/index.html')),
# Vite builds with base: '/dashboard/' → assets load from /dashboard/assets/
re_path(r'^dashboard/assets/(?P<path>.*)$', serve,
    {'document_root': settings.STATIC_ROOT / 'dashboard' / 'assets'}),
re_path(r'^media/(?P<path>.*)$', serve, {'document_root': settings.MEDIA_ROOT}),
re_path(r'^static/(?P<path>.*)$', serve, {'document_root': settings.STATIC_ROOT}),
```

## Multi-service (Bot + Celery) روی Railway

هر سرویس از همون repo، Start Command فرق داره:

| سرویس | Start Command |
|-------|--------------|
| web | `cd website && gunicorn website.wsgi:application --bind 0.0.0.0:$PORT --workers 2` |
| worker | `cd website && celery -A website.celery_app worker --loglevel=info` |
| bot | `cd website && python store/bot.py` |

Redis → **New → Database → Redis** — `REDIS_URL` خودکار inject میشه.

## Static/Media در Git (ephemeral disk)
Railway disk موقته — هر restart پاک. راه‌حل رایگان:
- `staticfiles/` رو از `.gitignore` حذف کن و commit کن
- `media/` رو commit کن (فایل‌های بالای 100MB حذف کن اول)
- `python-dotenv` حتماً توی `requirements.txt` باشه

## Pitfalls
- **`ModuleNotFoundError: No module named 'dotenv'`** → `python-dotenv` به requirements.txt اضافه کن
- **CSRF error** → `CSRF_TRUSTED_ORIGINS` env var ست کن (URL دقیق سرویس)
- **داشبورد سفید (blank)** → Vite با `base: '/dashboard/'` build می‌کنه؛ assets از `/dashboard/assets/` لود می‌شن؛ بدون `re_path` mapping → 404 → Django HTML 404 برمی‌گردونه → مرورگر MIME error می‌ده (text/html not valid stylesheet). راه‌حل: `re_path` بالا.
- **عکس‌ها نیست** → `MEDIA_ROOT = "/var/www/resin-media"` hardcode بود؛ Railway این path ندار → داینامیک کن
- **پنل ادمین بدون CSS** → `collectstatic` نشده یا `staticfiles/` commit نشده
- **Session cookie ست نمیشه** → `SESSION_COOKIE_SECURE=True` + HTTP = reject. env var `HTTPS=true` ست کن روی Railway
- **Celery Redis connection fail** → `redis://127.0.0.1:6379` روی Railway وجود نداره؛ باید `REDIS_URL` باشه
- **Bot crash** → path اشتباه؛ `cd website && python store/bot.py` (نه `python bot.py`)
- **Render requires credit card** → Railway جایگزین کن
- **Node 20.18 روی nixpacks** → vite 8 / rolldown نیاز به `^20.19 || >=22` داره؛ `nodejs_20` نصب 20.18 می‌کنه → build crash. راه‌حل: `nixpacks.toml` رو به `nodejs_22` تغییر بده
- **React API hardcode** → اگه `const API = 'https://main-server.com/api'` باشه، داشبورد روی Railway به سرور اصلی وصل میشه نه backend خودش. راه‌حل: `const API = '/api'` (relative path — همیشه به همون host می‌زنه). `import.meta.env.VITE_API_URL` کار نمی‌کنه چون vite string literal انتظار داره نه runtime expression — مستقیم `/api` بنویس.
- **Vite cache pitfall** → source `App.jsx` درستیه ولی `dist/assets/*.js` هنوز old URL داره. دلیل: vite transform cache. راه‌حل: `rm -rf node_modules/.vite dist && npm run build`. بعد از build با `grep 'hardcoded-url' dist/assets/*.js` تأیید کن که نیست.
- **Built dist در git stale** → `staticfiles/dashboard/assets/index-OLD.js` commit شده ولی new file `index-NEW.js` جداگانه commit شده → هر دو توی git می‌مونن؛ مرورگر old file رو load می‌کنه چون `index.html` هنوز اسم old رو reference داره. راه‌حل: `rm -rf website/staticfiles/dashboard && cp -r admin-frontend/dist website/staticfiles/dashboard` — پوشه رو کامل جایگزین کن نه merge.
- **Vite Native Binding / Rolldown Error**: خطای `Cannot find native binding` روی پلتفرم لینوکس به علت تداخل فایلهای قفل شده ویندوز/مک رخ می‌دهد. راه‌حل: حذف دستی `package-lock.json` و `node_modules` قبل از اجرای `npm install` در اسکریپت بیلد به شکل زیر:
  `rm -f admin-frontend/package-lock.json && cd admin-frontend && npm install && npm run build`
- **Railway Build Cache Pitfall**: اگر کارهای فوق را کردید و باز همان ارور قبلی را گرفتید، ریپورت بیلد به علت کش سیستم مجدد کرش می‌کند. راه‌حل: در پنل تنظیمات دیپلویمنت Railway گزینه **Redeploy without cache** را بزنید یا کش بیلد را کلیر کنید.
- **Nginx Alias vs World-readable path**: اگر عکس‌ها ۴۰۴ می‌دهند چک کنید ان‌جینکس از کدام مسیر به عنوان مستعار (Alias) در حال خواندن است و حتماً فایلهای جدید در همان مسیر فیزیکی کپی شوند. (مثال: تفاوت مسیر `/var/www/resin-media` و `/root/resin-web/website/media` که برطرف شد).
- **Hero-to-Section Background Transition Spacing**: وقتی رنگ پس‌زمینه هیرو با سکشن بعدی متفاوت است، چسبیدن مرز دو سکشن دیزاین را خفه می‌کند. افزودن `mb-16 sm:mb-24` به سکشن هیرو، فضای تنفس و تفکیک بصری عالی ایجاد می‌کند.
- **Django Template Memory Cache in Gunicorn**: تغییر فایل‌های `base.html` یا `home.html` روی دیسک گاهی توسط Gunicorn کش باقی می‌ماند و در لایو دیده نمی‌شود. حتماً فرآیند gunicorn با `pkill -f 'gunicorn website.wsgi'` کلاً ری‌استارت شده و با `curl -s http://localhost:8000/ | grep "new_string"` خروجی محلی چک شود.
- **Mahgol Resin Palette Standard**: رنگ اصلی تثبیت‌شده برند مه‌گل بنفش سلطنتی `#6809A2` است (جایگزین صورتی‌های جیغ و بنفش‌های دودی کدر). ترکیب:
  - رنگ اصلی اکشن/دکمه‌ها: `#6809A2` (Royal Purple)
  - گرادیان شاداب: `#9D09A2`
  - ورق طلا / جزئیات: `#FFB800` (Sunny Gold)
  - پس‌زمینه خنثی: `#FBF8FD` (یاسی بسیار کمرنگ و شاداب)
  - فوتر تیره و سنگین: `#140321` (بادمجانی عمیق)

