# Cart Quantity +/− 500 Error — Full Debugging Session

## Symptom

After clicking `+` button in cart, user gets HTTP 500. Cart page renders 200 OK when first loaded, but any quantity change blows up.

## Root Cause

### Primary: `get_object_or_404(CartItem, id=item_id, cart=cart)`

The original `update_cart_quantity` used:

```python
item = get_object_or_404(CartItem, id=item_id, cart=cart)
```

This silently 404s (which renders as 500 in browser) when:
- User is a **guest** whose session changed between page load and button click
- User's `get_or_create_cart` created a **new Cart** (different `id`)
- The `CartItem` with `id=item_id` exists but belongs to the OLD Cart

### Secondary: Guest cart with `session_key=None`

```python
# BAD: creates new Cart every time for unauthenticated users
session_key = request.session.session_key  # None!
cart, created = Cart.objects.get_or_create(session_key=session_key)
# → duplicates multiply until 500
```

## Fix

### 1. `get_or_create_cart` — ensure session_key is never None

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
            # Fallback — should never happen after session.create()
            cart, created = Cart.objects.get_or_create(id=99999)
    return cart
```

### 2. `update_cart_quantity` — find item by ID, then migrate

```python
def update_cart_quantity(request, item_id, delta):
    cart = get_or_create_cart(request)
    try:
        item = CartItem.objects.get(id=item_id)
    except CartItem.DoesNotExist:
        return redirect('store:cart_view')

    # Migrate item to current cart if needed
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

    # Apply limits
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

## Verification Command

```bash
# Add product to cart
curl -s -c /tmp/cookies.txt "http://localhost:8000/cart/add/1000/" -D -
# Get cart HTML, extract item_id
curl -s -b /tmp/cookies.txt "http://localhost:8000/cart/" | grep -oP 'href="/cart/qty/\d+/1/"'
# Hit the + button
curl -s -b /tmp/cookies.txt -D - "http://localhost:8000/cart/qty/19/1/" | head -3
# Should return 302 redirect to /cart/
```

## Duplicates Check

```sql
SELECT user_id, session_key, COUNT(*) 
FROM store_cart 
GROUP BY user_id, session_key 
HAVING COUNT(*) > 1;
```

Clean up with:

```python
from django.db.models import Count
from store.models import Cart
dups = Cart.objects.values('user', 'session_key').annotate(cnt=Count('id')).filter(cnt__gt=1)
# Inspect and manually delete extras
```
