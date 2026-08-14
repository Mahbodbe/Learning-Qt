# Unified JWT + Django Session Login (React Admin Panel)

## Problem
React SPA admin uses JWT. Main Django site uses session cookies. Two separate logins = user confusion.

## Solution: Single `/api/admin-login/` endpoint

Issues both JWT (for React) and Django session cookie (for main site). One login = both synced.

### Backend (`store/views.py`)

```python
from rest_framework.decorators import api_view, permission_classes
from rest_framework.permissions import AllowAny
from rest_framework.response import Response
from rest_framework_simplejwt.tokens import RefreshToken
from django.contrib.auth import authenticate, login as django_login

@api_view(['POST'])
@permission_classes([AllowAny])
def admin_login(request):
    username = request.data.get('username', '')
    password = request.data.get('password', '')
    # CRITICAL: request._request — DRF wraps Django HttpRequest, must unwrap
    user = authenticate(request._request, username=username, password=password)
    if user is None:
        return Response({'detail': 'نام کاربری یا رمز اشتباه است'}, status=401)
    if not (user.is_staff or user.is_superuser):
        return Response({'detail': 'دسترسی مجاز نیست'}, status=403)
    django_login(request._request, user)   # sets Django session cookie
    refresh = RefreshToken.for_user(user)
    return Response({
        'access': str(refresh.access_token),
        'refresh': str(refresh),
        'username': user.username,
    })
```

### Register in `api_urls.py`

```python
from .views import admin_login
urlpatterns = [
    path('admin-login/', admin_login, name='admin-login'),
] + router.urls
```

### React axios interceptor (auto-logout on 401)

```javascript
const api = axios.create({ baseURL: API });
api.interceptors.request.use(cfg => {
  const t = localStorage.getItem('access');
  if (t) cfg.headers.Authorization = `Bearer ${t}`;
  return cfg;
});
api.interceptors.response.use(r => r, async err => {
  if (err.response?.status === 401) {
    localStorage.removeItem('access');
    window.location.reload();  // prevents infinite loop on JWT expiry
  }
  return Promise.reject(err);
});
```

### Pitfalls
- `request` vs `request._request` — always unwrap for `authenticate()` and `django_login()`
- Only `is_staff` or `is_superuser` users should access admin panel; reject others with 403
- 401 interceptor auto-reload prevents loop when JWT expires mid-session

---

# ProductImage REST API (separate upload endpoint)

```python
class ProductImageViewSet(viewsets.ModelViewSet):
    queryset = ProductImage.objects.all()
    serializer_class = ProductImageSerializer
    parser_classes = [MultiPartParser, FormParser]

    def get_queryset(self):
        product_id = self.request.query_params.get('product')
        qs = super().get_queryset()
        if product_id:
            qs = qs.filter(product_id=product_id)
        return qs
```

Register: `router.register(r'product-images', ProductImageViewSet, basename='productimage')`

Upload: `FormData` with `product=<id>`, `image=<file>`, `is_primary=true/false`

Set primary: `PATCH /api/product-images/<id>/` with `{is_primary: true}` — also patch others to set `is_primary: false`.

---

# React Admin SPA — File Transfer to Server

## Problem
`heredoc` over SSH for JSX files breaks when single quotes appear inside the content
(shell's `<< 'EOF'` delimiter chokes on `'` in JSX).

## Solution: write_file local → scp

```
1. write_file locally: /tmp/App.jsx  (or /tmp/chunk1.jsx, chunk2.jsx, ...)
2. sshpass scp /tmp/App.jsx root@server:/root/resin-web/admin-frontend/src/App.jsx
3. ssh ... "cd /root/resin-web/admin-frontend && npm run build"
```

For large JSX files: split into logical sections (Login, Products, Types, etc.),
append each with `cat >> file << 'JSXEOF'` directly on server (works as long as
the delimiter itself doesn't appear in the content).

## Gunicorn reload without systemd (nohup mode)

```bash
# find master pid
pgrep -af gunicorn | grep -v worker | head -1

# graceful reload (workers restart one by one, no downtime)
kill -HUP <master_pid>

# verify new code is live
curl -s -o /dev/null -w '%{http_code}' http://localhost:8000/api/admin-login/ -X POST
# expect 401 (endpoint exists, no credentials sent)
```
