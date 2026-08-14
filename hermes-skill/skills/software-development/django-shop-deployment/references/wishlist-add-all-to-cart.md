# Wishlist → Cart "Add All" Button

## Pattern

Add a single button at the top of the wishlist page that moves ALL wishlisted items into the current cart at once.

### View

```python
@login_required
@csrf_exempt
def add_all_wishlist_to_cart(request):
    if request.method != 'POST':
        return JsonResponse({'error': 'POST required'}, status=405)
    cart = get_or_create_cart(request)
    wishlist_items = Wishlist.objects.filter(user=request.user).select_related('product')
    for w in wishlist_items:
        CartItem.objects.get_or_create(cart=cart, product=w.product)
    return JsonResponse({'status': 'ok'})
```

### URL

```python
path("wishlist/add-all/", views.add_all_wishlist_to_cart, name="add_all_wishlist_to_cart"),
```

### Template (wishlist.html)

```django
{% if products %}
<button onclick="addAllWishlistToCart()" class="...">
  افزودن همه به سبد خرید
</button>
{% endif %}

<script>
function addAllWishlistToCart() {
  fetch('/wishlist/add-all/', {method: 'POST', headers: {'X-CSRFToken': '{{ csrf_token }}'}})
    .then(r => {
      if (r.ok) {
        updateCartBadge();
        window.location.href = '{% url "store:cart_view" %}';
      }
    });
}
</script>
```

UpdateCartBadge() is defined in base.html and calls `/cart/count/` then updates `#cart-badge` text content.

## Wishlist page also gets AJAX add-to-cart

The "+ سبد" button on each wishlist card should call `addToCart(code, this)` (same global function from base.html), not `sendToBot()`, for the same smooth no-refresh experience.
