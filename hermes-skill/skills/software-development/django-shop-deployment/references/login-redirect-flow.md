# Login Redirect Flow with `next` Parameter Preservation

## Problem
When a user clicks "Order" on a product page without being logged in, they should:
1. Be redirected to login with `?next=/product/<code>/` parameter  
2. After successful login, be redirected back to the product page (`/product/<code>/`)
3. Click "Order" again → POST to `/order/<code>/` → success

## Root Cause of Broken Flow
The login form was **losing the `next` parameter during POST** because:
1. Form action was `action="{% url 'store:user_login' %}"` — missing `?next=` parameter
2. No hidden `next` input in the form  
3. When form submitted via POST, `request.GET.get('next')` returned `None`
4. Signup link also didn't preserve `?next=`

## Solution — Three Parts Must All Be Fixed

### 1. Form Action Must Preserve `?next=`
```html
<!-- BAD: loses next on POST -->
<form method="post" action="{% url 'store:user_login' %}">

<!-- GOOD: preserves next through POST -->
<form method="post" action="{% url 'store:user_login' %}{% if request.GET.next %}?next={{ request.GET.next|urlencode }}{% endif %}">
```

### 2. Hidden Input for `next` (Backup)
```html
<input type="hidden" name="next" value="{{ request.GET.next|default:'' }}">
```

### 3. Signup Link Must Also Preserve `next`
```html
<a href="{% url 'store:user_signup' %}{% if request.GET.next %}?next={{ request.GET.next|urlencode }}{% endif %}">
```

### 4. View Must Read `next` from POST and GET
```python
def user_login(request):
    if request.method == 'POST':
        # CRITICAL: Check POST first (form submission), then GET (direct link)
        next_url = request.POST.get('next') or request.GET.get('next')
        if next_url:
            return redirect(next_url)
        return redirect('store:product_list')
```

Same for `user_signup`:
```python
def user_signup(request):
    if request.method == 'POST':
        ...
        login(request, user)
        next_url = request.POST.get('next') or request.GET.get('next')
        if next_url:
            return redirect(next_url)
        return redirect('store:product_list')
```

### 5. JavaScript `sendToBot` Must Pass Correct `next`
```javascript
function sendToBot(code) {
  {% if user.is_authenticated %}
    // existing logic
  {% else %}
  const currentProductPath = `/product/${code}/`;
  window.location.href = `{% url 'store:user_login' %}?next=${encodeURIComponent(currentProductPath)}`;
  {% endif %}
}
```

### 6. Confirmation Page Endpoint
After a successful order POST, the JS redirects to `/order/confirm/`:
```python
# urls.py
path("order/confirm/", views.order_confirmation, name="order_confirm"),

# views.py
def order_confirmation(request):
    return render(request, 'store/bot_order_sent.html', {'code': request.GET.get('code', '')})
```

DO NOT use `/bot_order_sent` as a URL path — it doesn't exist as a view and will return 404.

## Verification Checklist
After implementing, verify the full flow:
1. Go to `/product/7001/` (unauthenticated)
2. Click "سفارش" → redirects to `/login/?next=/product/7001/`
3. Login with credentials → redirects back to `/product/7001/`
4. Click "سفارش" again → POST to `/order/7001/` → shows success page at `/order/confirm/`

## Debugging Tip
Add temporary logging in `user_login`:
```python
print(f"DEBUG: next_url from POST={request.POST.get('next')}, GET={request.GET.get('next')}")
```

## Pitfalls

1. **Using only `request.GET.get('next')`** — breaks because `next` is sent via POST body (hidden input), not URL. Must check POST first.
2. **Missing hidden input** — form won't carry `next` through POST.
3. **JS uses bare `/order/${code}`** (no trailing slash) — Django redirect view may strip or modify it. Always use trailing slash.
4. **Browser cache of old JS** — after fixing, user must Hard Refresh (Ctrl+Shift+R) to get the updated `sendToBot` function. Add `?v=` to CSS/JS URLs to force cache busting.
5. **Signup link without `?next=`** — user goes through signup then lands on product_list, not the product they came from. Must preserve `?next=` in both login AND signup links.
6. **Missing `/order/confirm/` route** — JS redirects to `/bot_order_sent` after successful POST, but that route doesn't exist → 404. Create a proper confirmation endpoint.
