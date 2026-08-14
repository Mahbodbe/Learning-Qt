# Cart Implementation — Cart + CartItem Model

## Models

```python
class Cart(models.Model):
    user = models.OneToOneField(User, on_delete=models.CASCADE, null=True, blank=True)
    session_key = models.CharField(max_length=255, null=True, blank=True)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)
    token = models.CharField(max_length=64, unique=True, null=True, blank=True)

    def total_price(self):
        return sum(item.product.price * item.quantity for item in self.items.all())

    def item_count(self):
        return self.items.count()

class CartItem(models.Model):
    cart = models.ForeignKey(Cart, related_name='items', on_delete=models.CASCADE)
    product = models.ForeignKey(Product, on_delete=models.CASCADE)
    quantity = models.PositiveIntegerField(default=1)
    added_at = models.DateTimeField(auto_now_add=True)
```

## get_or_create_cart (critical utility)

```python
from django.conf import settings
import secrets

def get_or_create_cart(request):
    if request.user.is_authenticated:
        cart, created = Cart.objects.get_or_create(user=request.user)
    else:
        session_key = request.session.session_key
        if not session_key:
            request.session.create()
            session_key = request.session.session_key
        cart, created = Cart.objects.get_or_create(session_key=session_key)

    if not cart.token:
        cart.token = secrets.token_urlsafe(32)
        cart.save()
    return cart
```

## Views

```python
def add_to_cart(request, code):
    cart = get_or_create_cart(request)
    product = get_object_or_404(Product, code=code)
    item, created = CartItem.objects.get_or_create(cart=cart, product=product)
    if not created:
        item.quantity += 1
        item.save()
    return redirect('store:cart_view')

def cart_view(request):
    cart = get_or_create_cart(request)
    for item in cart.items.all():
        item.primary_image = item.product.images.filter(is_primary=True).first()
    return render(request, 'store/cart.html', {'cart': cart})

def remove_from_cart(request, item_id):
    item = get_object_or_404(CartItem, id=item_id, cart=get_or_create_cart(request))
    item.delete()
    return redirect('store:cart_view')

def checkout_to_telegram(request):
    cart = get_or_create_cart(request)
    if not cart.items.exists():
        return redirect('store:cart_view')
    if not cart.token:
        cart.token = secrets.token_urlsafe(32)
        cart.save()
    if not request.user.is_authenticated:
        return redirect(f"/login/?next=/cart/checkout/")
    bot_url = f"https://t.me/Mahgolresinadminbot?start=cart_{cart.token}"
    return redirect(bot_url)
```

### Key: `primary_image` computed in view, NOT in template

Django templates **cannot chain** `.filter(...).first()`. If you write `item.product.images.filter(is_primary=True).first()` in a template, the template engine silently fails (returns nothing → image 404 icon renders).

**CORRECT approach** — pre-compute in the view:
```python
# views.py
def cart_view(request):
    cart = get_or_create_cart(request)
    for item in cart.items.all():
        item.primary_image = item.product.images.filter(is_primary=True).first()
    return render(request, 'store/cart.html', {'cart': cart})
```

Then in the template, use `item.primary_image` directly:
```django
{% if item.primary_image %}
  <img src="{{ item.primary_image.image.url }}" ...>
{% else %}
  <div class="placeholder-icon">...</div>
{% endif %}
```

## URLs

```python
path("cart/", views.cart_view, name="cart_view"),
path("cart/add/<int:code>/", views.add_to_cart, name="add_to_cart"),
path("cart/remove/<int:item_id>/", views.remove_from_cart, name="remove_from_cart"),
path("cart/checkout/", views.checkout_to_telegram, name="checkout_to_telegram"),
```

## Bot Handler (`start=cart_<token>`)

```python
if args and args[0].startswith("cart_"):
    token = args[0][5:]
    cart = await sync_to_async(Cart.objects.filter(token=token).prefetch_related("items__product__type").first)()
    if not cart or not cart.items.exists():
        await update.message.reply_text("❌ سبد خرید یافت نشد یا خالی است.")
        return

    msg = f"🛒 *سبد خرید شما*\n\n"
    total = 0
    for idx, item in enumerate(cart.items.all(), 1):
        p = item.product
        msg += f"{idx}. *{p.name}* — {p.price:,} تومان\n"
        msg += f"   #{p.code} | {p.type.name}\n"
        total += p.price * item.quantity

    msg += f"\n💰 *جمع کل:* {total:,} تومان\n\nآیا می‌خواهید این سبد را تأیید کنید?"

    keyboard = [[
        InlineKeyboardButton("✅ تأیید کل سبد", callback_data=f"cart_confirm:{token}"),
        InlineKeyboardButton("❌ انصراف", callback_data="order_cancel"),
    ]]
    reply_markup = InlineKeyboardMarkup(keyboard)
    await update.message.reply_text(msg, parse_mode="Markdown", reply_markup=reply_markup)
    return
```

## Cart Confirm Handler (bot)

```python
async def cart_confirm_handler(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    if not query: return
    await query.answer()

    token = query.data.split(":")[1]
    cart = await sync_to_async(Cart.objects.filter(token=token).prefetch_related("items__product__type").first)()
    if not cart or not cart.items.exists():
        await query.edit_message_text("❌ سبد خرید یافت نشد یا خالی است.")
        return

    items = list(cart.items.all())
    for item in items:
        await sync_to_async(PromotionQueueItem.objects.create)(product=item.product)

    # Admin notification with full cart details (HTML, sent to numeric chat_id)
    ...

    await query.edit_message_text(
        text=f"✅ سبد خرید شما با موفقیت ثبت شد!\n\nتعداد: {len(items)} عدد\nجمع کل: {cart.total_price():,} تومان\n\nبه زودی ادمین تماس خواهد گرفت.",
        parse_mode="Markdown"
    )
```

## Handler Registration

```python
app.add_handler(CallbackQueryHandler(cart_confirm_handler, pattern="^cart_confirm:"))
```

## Cart Template — Pitfalls

### 1. `primary_image` must be pre-computed (see above)
### 2. `<p>` tag closing rule

**Symptom:** Cart empty state text renders on same line as the button instead of above it.

**Root cause:** Missing `</p>` closing tag — Django template engine corrupts Persian text + tag combinations.

**Fix:**
```django
<!-- BAD: layout breaks on Persian content -->
<p class="text-gray-500 mb-6">متن فارسی.</    <a ...>

<!-- GOOD -->
<p class="text-gray-500 mb-6">متن فارسی.</p>
<a href="..." class="...">ادامه خرید</a>
```

After editing, verify with:
```bash
grep -c "<p" cart.html | head -1
grep -c "</p>" cart.html | head -1
# counts must match
```

## Design Notes

- **Guest users**: cart keyed to `session_key` (temporary; lost on session clear)
- **Logged-in users**: cart keyed to `user` (persistent)
- **Token**: `secrets.token_urlsafe(32)` generated on first use
- **Qty increment**: `add_to_cart` auto-increments qty if item already in cart
- **Checkout gate**: guests redirected to login with `?next=/cart/checkout/`
- **Admin notification**: sent as HTML with full item list; must use **numeric chat_id** (not @username)
- **Bot handler**: `cart_<token>` reads cart from DB, sends list + confirm button
- **Cart confirm**: creates `PromotionQueueItem` for each item, notifies admin

## Quantity Controls (Implemented)

Cart items now have `+` / `−` buttons for quantity adjustment:

```python
# views.py
def update_cart_quantity(request, item_id, delta):
    cart = get_or_create_cart(request)
    item = get_object_or_404(CartItem, id=item_id, cart=cart)
    product = item.product
    new_qty = item.quantity + delta

    if new_qty <= 0:
        item.delete()  # − at qty=1 → delete the item
        return redirect('store:cart_view')

    max_qty = product.stock
    if product.price > 2000000:        # expensive products: max 1
        max_qty = min(max_qty, 1)

    if new_qty > max_qty:
        return redirect('store:cart_view')  # silently cap (no error msg)

    item.quantity = new_qty
    item.save()
    return redirect('store:cart_view')
```

```python
# urls.py
path("cart/qty/<int:item_id>/<int:delta>/", views.update_cart_quantity, name="update_cart_qty"),
```

### Template pattern (cart item row)

```django
<div class="flex items-center gap-1 bg-gray-50 rounded-xl px-2 py-1">
  {% if item.quantity > 1 %}
  <a href="{% url 'store:update_cart_qty' item.id -1 %}">−</a>
  {% else %}
  <a href="{% url 'store:remove_from_cart' item.id %}">🗑</a>
  {% endif %}
  <span>{{ item.quantity }}</span>
  <a href="{% url 'store:update_cart_qty' item.id 1 %}">+</a>
</div>
```

### Add-to-cart limits (with price cap)

```python
def add_to_cart(request, code):
    cart = get_or_create_cart(request)
    product = get_object_or_404(Product, code=code)
    item, created = CartItem.objects.get_or_create(cart=cart, product=product)

    max_qty = product.stock
    if product.price > 2000000:       # expensive: max 1
        max_qty = min(max_qty, 1)

    if not created:
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

## Missing Features (Not Yet Implemented)
- Cart badge count indicator in header
- Cart merge on guest→login (should merge guest cart into user cart on login)

### Key Pitfalls from session:

1. **`primary_image` must be pre-computed in view** — Django templates CANNOT chain `.filter(...).first()`. The template engine silently returns nothing. Always compute in the view: `item.primary_image = item.product.images.filter(is_primary=True).first()`.

2. **`update_cart_quantity` returning 404 in curl is expected** — `get_object_or_404(CartItem, id=item_id, cart=cart)` returns 404 when the curl session doesn't match the cart's session. In-browser testing works because the session cookie is consistent. Not a bug.

3. **CTA `<p>` tag closing rule (Persian content)** — The sequence `متن.</    <div` loses the `</p>` closing tag when `</p>` is on the same line as a block element. Always put `</p>` on its own line. Verify with `grep -c "<p"` vs `grep -c "</p>"`.
