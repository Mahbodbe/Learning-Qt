# Cart Guest-to-Authenticated Migration

## Problem

When a guest (non-logged-in) user adds items to cart, the cart is stored with `session_key`. When they log in, a **new** cart is created for the user (via `get_or_create(user=user)`), so the guest items are **lost** — the cart appears empty after login.

## Fix: Merge on Login

In `user_login` view, immediately after `login(request, user)`:

```python
def user_login(request):
    if request.method == 'POST':
        ...
        if user is not None:
            login(request, user)
            
            # ---- BEGIN CART MERGE ----
            session_key = request.session.session_key
            if session_key:
                guest_cart = Cart.objects.filter(session_key=session_key).first()
                if guest_cart and guest_cart.items.exists():
                    user_cart, _ = Cart.objects.get_or_create(user=user)
                    for item in guest_cart.items.all():
                        existing = CartItem.objects.filter(cart=user_cart, product=item.product).first()
                        if existing:
                            existing.quantity += item.quantity
                            existing.save()
                        else:
                            item.cart = user_cart
                            item.save()
                    guest_cart.delete()
            # ---- END CART MERGE ----
            
            return redirect('store:product_list')
```

Do **NOT** merge in `get_or_create_cart()` — that function is called on every request and merging there would cause race conditions. Merge only at the login boundary.

## Verified Behavior

1. Guest adds product(s) to cart → items stored under `session_key`
2. Guest clicks "سفارش در تلگرام" → redirected to login with `?next=/cart/checkout/`
3. User logs in → guest cart items transferred to user cart
4. User lands on cart page → items are there, not lost

## Edge Case: Signup

Same merge logic applies in `user_signup` after `login(request, user)` — but in practice a brand-new user has no existing user cart to merge into, so the `get_or_create(user=user)` creates one and `guest_cart.items` migrate into it cleanly.
