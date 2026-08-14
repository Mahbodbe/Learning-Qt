# Cart AJAX + Badge Implementation

Session: 2026-07-10. User wanted add-to-cart to work without page reload (AJAX) + cart badge count in header.

## Key changes

### 1. AJAX addToCart function (base.html)

```javascript
function addToCart(code, btn) {
  fetch('/cart/add/' + code + '/', {method: 'POST', headers: {'X-CSRFToken': '{{ csrf_token }}'}})
    .then(r => { if (r.ok) updateCartBadge(); })
    .catch(() => {});
}

function updateCartBadge() {
  fetch('/cart/count/', {method: 'GET'})
    .then(r => r.json())
    .then(d => {
      let badge = document.getElementById('cart-badge');
      if (badge) { badge.textContent = d.count; badge.classList.remove('hidden'); }
    })
    .catch(() => {});
}
```

### 2. Cart count endpoint (views.py)

```python
def cart_count(request):
    cart = get_or_create_cart(request)
    return JsonResponse({'count': cart.item_count()})
```

URL: `path("cart/count/", views.cart_count, name="cart_count")`

### 3. Badge HTML in header (base.html)

```html
<a href="{% url 'store:cart_view' %}" class="...relative">
  سبد خرید
  <span id="cart-badge" class="absolute -top-1.5 -right-1.5 bg-rose-500 text-white text-[10px] rounded-full px-1.5 py-0.5 min-w-[16px] hidden"></span>
</a>
```

### 4. Button change in product cards

From `<a href="{% url 'store:cart_add' p.code %}">` to `<button onclick="addToCart({{ p.code }}, this)">`.

## Critical Gotcha: URL name mismatch → silent 500

Template wrote `{% url 'store:cart_add_one' item.product.code %}` but URL was named `cart_add` (not `cart_add_one`). Django `NoReverseMatch` at render time → 500 with no visible traceback. 

**Fix:** `grep -E "url 'store:(cart|checkout)" templates/` and cross-check URL names in `urls.py`. Every rename in urls.py must be mirrored in ALL templates.

## Badge not updating after add

Make sure:
1. `cart_count` view returns `JsonResponse({'count': N})` 
2. JS fetch doesn't require auth (guests can also have carts)
3. Cart item is actually created (check DB with `CartItem.objects.all()`)
