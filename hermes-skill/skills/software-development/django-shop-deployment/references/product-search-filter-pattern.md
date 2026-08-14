# Product Filter + Sort Dropdowns + Home Page Structure

## Page Architecture (for this user)

```
/            → Home page: Header → Hero → 3 featured product cards → Footer
/products/   → Full catalog: Search bar + Category dropdown + Sort dropdown + Product grid + CTA
```

## View with sort + search

```python
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
    
    # Apply sorting
    if sort_by == "price_asc":
        qs = qs.order_by("price")
    elif sort_by == "price_desc":
        qs = qs.order_by("-price")
    elif sort_by == "stock":
        qs = qs.order_by("-stock", "-id")
    else:  # newest (default)
        qs = qs.order_by("-id")
    
    return render(request, "store/product_list.html", {..., "current_sort": sort_by})
```

## Dropdown Categories (not pills)

Avoid inline pill-style category buttons. Use dropdown menus:

```django
<!-- Category Dropdown -->
<div class="relative">
  <button onclick="this.nextElementSibling.classList.toggle('hidden')" 
          class="flex items-center gap-2 px-4 py-2 bg-white border rounded-lg text-sm">
    <span>دسته‌بندی: {{ active_type|default:"همه" }}</span>
  </button>
  <div class="hidden absolute top-full right-0 mt-1 w-52 bg-white border rounded-lg shadow-lg z-20">
    <a href="?sort={{ current_sort }}">همه</a>
    {% for t in types %}
    <a href="?type={{ t.name|urlencode }}&sort={{ current_sort }}">{{ t.name }}</a>
    {% endfor %}
  </div>
</div>

<!-- Sort Dropdown -->
<div class="relative">
  <button onclick="this.nextElementSibling.classList.toggle('hidden')" class="...">
    <span>مرتب‌سازی</span>
  </button>
  <div class="hidden absolute ...">
    <a href="?sort=newest">جدیدترین</a>
    <a href="?sort=price_asc">قیمت: کم به زیاد</a>
    <a href="?sort=price_desc">قیمت: زیاد به کم</a>
    <a href="?sort=stock">موجودی</a>
  </div>
</div>
```

## Home Page

Home is separate from /products/. Never redirect to products.

Structure: Header → Hero Section → 2-3 Product Cards → CTA → Footer

```python
def home(request):
    featured = Product.objects.select_related("type").order_by("-id")[:3]
    for p in featured:
        p.primary_image = p.images.filter(is_primary=True).first()
    return render(request, "store/home.html", {"featured_products": featured})
```

## CTA Section Pitfall

The `<p>` tag's closing `</p>` is frequently eaten by template formatting. Verify:

```django
<p class="mt-2 text-violet-200 max-w-lg mx-auto">متن.</p>
<!-- closing </p> is REQUIRED, next line must have <div> not more text -->
<div class="mt-6 ...">...</div>
```

## Logo Debug (Quick Chart)

| URL | Expected | Remark |
|-----|----------|--------|
| `http://domain/media/logo.jpg` | 200 via nginx | production check |
| `http://localhost:8000/media/logo.jpg` | 404 | django doesn't serve media in DEBUG=False |
