# Unified JWT + Django Session Login (React Admin Panel Pattern)

## Problem
React admin panel on `/dashboard/` needs:
1. JWT for DRF API calls (`Authorization: Bearer ...`)
2. Django session cookie so the main site (`/`) also considers the user logged in

Naive approach (two separate logins, or JWT only) breaks the "one login = everywhere" requirement.

## Solution: Plain Django View (NOT @api_view)

`@api_view` + `django_login()` ست می‌کنه ولی `Set-Cookie: sessionid` رو توی response header نمی‌ذاره — DRF response از `request._request` cookie رو pick نمی‌کنه. Session middleware باید response رو process کنه.

**Fix — use `@csrf_exempt` plain Django view:**

```python
# store/views.py
from django.views.decorators.csrf import csrf_exempt
from django.http import JsonResponse
from django.contrib.auth import authenticate, login as django_login
from rest_framework_simplejwt.tokens import RefreshToken

@csrf_exempt
def admin_login(request):
    import json as _json
    if request.method != 'POST':
        return JsonResponse({'detail': 'POST required'}, status=405)
    try:
        body = _json.loads(request.body)
    except Exception:
        return JsonResponse({'detail': 'invalid json'}, status=400)

    username = body.get('username', '')
    password = body.get('password', '')
    user = authenticate(request, username=username, password=password)
    if user is None:
        return JsonResponse({'detail': 'نام کاربری یا رمز اشتباه است'}, status=401)
    if not (user.is_staff or user.is_superuser):
        return JsonResponse({'detail': 'دسترسی مجاز نیست'}, status=403)

    django_login(request, user)   # ← sets session via middleware (works in plain view)

    refresh = RefreshToken.for_user(user)
    return JsonResponse({
        'access': str(refresh.access_token),
        'refresh': str(refresh),
        'username': user.username,
    })


@csrf_exempt
def admin_logout(request):
    from django.contrib.auth import logout as django_logout
    django_logout(request)
    return JsonResponse({'detail': 'logged out'})
```

```python
# api_urls.py — register outside DRF router
from django.urls import path
from .views import admin_login, admin_logout
urlpatterns = [
    path('admin-login/', admin_login, name='admin-login'),
    path('admin-logout/', admin_logout, name='admin-logout'),
] + router.urls
```

**Verify** — response must have BOTH cookies:
```bash
curl -s -X POST https://domain/api/admin-login/ \
  -H 'Content-Type: application/json' \
  -d '{"username":"X","password":"Y"}' -v 2>&1 | grep Set-Cookie
# Expected:
# Set-Cookie: csrftoken=...
# Set-Cookie: sessionid=...; HttpOnly; Path=/; SameSite=Lax
```

## CORS: withCredentials + Django Main Site Forms

### Option A — Explicit origins (full isolation)
```python
# settings.py
CORS_ALLOW_ALL_ORIGINS = False
CORS_ALLOWED_ORIGINS = ["https://your-domain.com"]
CORS_ALLOW_CREDENTIALS = True
```
**Pitfall:** if main site Django forms also live on the same domain, their CSRF
still works — but only because they don't go through the CORS middleware.
No CORS header = no CORS block for same-origin form submissions.

### Option B — Wildcard origins scoped to /api/ only (RECOMMENDED for same-domain setups)
Allows wildcard CORS on API routes without breaking Django form CSRF:
```python
CORS_ALLOW_ALL_ORIGINS = True
CORS_URLS_REGEX = r'^/api/.*$'   # ← CORS only applies to /api/ routes
CORS_ALLOW_CREDENTIALS = True
```
This is safe: wildcard CORS only hits `/api/*`, not Django views that render forms.
Browser still sends `sessionid` cookie because requests are same-origin.

**CRITICAL:** NEVER flip `CORS_ALLOW_ALL_ORIGINS = False` on a site where Django form views and
React dashboard live on the same domain. When you change to explicit `CORS_ALLOWED_ORIGINS` list,
the CORS middleware starts enforcing CORS on ALL views — including Django login/order forms
(not just /api/). This causes CSRF middleware to reject form submissions with "CSRF cookie
not set" errors even though the form renders fine and succeeds on reconnect.
Fix: use Option B (wildcard + regex scoping) instead. If you must use explicit origins,
pair it with `CORS_URLS_REGEX` to limit enforcement to API routes.

**When Option B breaks (don't use it if):** you have API consumers from other domains
that need CORS + credentials simultaneously. Use Option A in that case.

## React/Axios: Login Must Use the Same `api` Instance

اگه login از `axios.post(...)` مستقیم بره (نه از `api` instance که `withCredentials: true` داره)، cookie ست نمی‌شه.

```js
// WRONG — axios.post مستقیم، withCredentials اعمال نمی‌شه
const r = await axios.post(`${API}/admin-login/`, { username, password });

// CORRECT — api instance داره withCredentials: true
const api = axios.create({ baseURL: API, withCredentials: true });
const r = await api.post('/admin-login/', { username, password });
```

## Unified Logout (React Panel)

داشبورد باید هم JWT پاک کنه هم session Django رو ببنده:

```js
// App.jsx — onLogout handler
async () => {
    localStorage.removeItem("access");
    localStorage.removeItem("username");
    try { await api.post('/admin-logout/', {}, { withCredentials: true }); } catch {}
    setAuth(false);
}
```

بدون صدا زدن `/admin-logout/`، session Django باقی می‌مونه و کاربر توی سایت اصلی
هنوز لاگینه حتی بعد از logout پنل.

## Gunicorn Reload (nohup, no systemd)

این سرور gunicorn رو بدون systemd با nohup اجرا می‌کنه:

```bash
# Reload (graceful restart workers):
kill -HUP $(pgrep -f 'gunicorn website.wsgi' | head -1)

# PID پیدا کردن:
pgrep -f 'gunicorn website.wsgi'
# اولین PID = master، بقیه = workers

# لاگ:
tail -f /tmp/gunicorn.log
```

**هرگز** `systemctl restart gunicorn` نزن — سرویس وجود نداره، 500 می‌گیری.

## Git Init on Production Server (First Push)

وقتی repo نیست و می‌خوای سرور رو به GitHub push کنی:

```bash
cd /root/resin-web
git init
git config user.email "admin@domain.com"
git config user.name "Admin"
```

**.gitignore حتماً باید شامل اینا باشه:**
```
venv/
__pycache__/
*.pyc
.env
db.sqlite3
/website/media/
/website/staticfiles/
/website/node_modules/
/admin-frontend/node_modules/
/admin-frontend/dist/
celerybeat-schedule.db
*.log
.DS_Store
```

`staticfiles/` و `node_modules/` رو قبل از `git add .` از index پاک کن:
```bash
git rm -r --cached website/staticfiles/ 2>/dev/null
git rm -r --cached website/node_modules/ 2>/dev/null
echo 'node_modules/' >> .gitignore
echo 'website/staticfiles/' >> .gitignore
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin https://<PAT>@github.com/<user>/<repo>.git
git push -u origin main
```

**Pitfall:** `git rm -r --cached` باید قبل از `git add .` باشه، وگرنه staged می‌مونن.
**Pitfall:** GitHub repo باید از قبل ساخته شده باشه — `git push` روی repo ناموجود fail می‌شه با `Repository not found`.
**Pitfall:** repo name might differ from what you think — check with `curl -s -H 'Authorization: token PAT' https://api.github.com/user/repos | python3 -c "import sys,json; [print(r['full_name']) for r in json.load(sys.stdin)]"` before setting remote. Pushing to `Mahbodbe/resin-web` when the real repo is `Mahbodbe/website` wastes a full round-trip.
