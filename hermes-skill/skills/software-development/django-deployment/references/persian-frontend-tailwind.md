# Persian/RTL Tailwind Frontend Patterns (for Iranian e-commerce)

## When to use
Building a frontend for Iranian users with RTL, Farsi fonts, and CDN-accessible Tailwind (no build pipeline needed).

## Base Template Structure

```html
<!DOCTYPE html>
<html lang="fa" dir="rtl">
<head>
  <script src="https://cdn.tailwindcss.com"></script>
  <link href="https://fonts.googleapis.com/css2?family=Vazirmatn:wght@300;400;500;600;700;800&display=swap" rel="stylesheet">
  <style>
    body { font-family: 'Vazirmatn', system-ui, sans-serif; }
    .product-card { transition: transform 0.3s ease, box-shadow 0.3s ease; }
    .product-card:hover { transform: translateY(-4px); }
  </style>
</head>
<body>
```

## Layout Components

### Sticky Header with Blur
```html
<header class="sticky top-0 z-50 bg-white/90 nav-blur border-b border-gray-100">
  <div class="max-w-7xl mx-auto px-4 flex items-center justify-between h-20">
    <a href="/" class="flex items-center gap-3">
      <div class="w-10 h-10 rounded-full bg-gradient-to-br from-violet-600 to-violet-800 flex items-center justify-center text-white font-bold">م</div>
      <span class="text-xl font-bold text-violet-700">نام فروشگاه</span>
    </a>
    <nav class="hidden md:flex items-center gap-8">
      <a href="/">خانه</a>
      <a href="/">محصولات</a>
    </nav>
    <button class="md:hidden p-2" onclick="document.getElementById('mobile-menu').classList.toggle('hidden')">
      <svg class="w-6 h-6" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 6h16M4 12h16M4 18h16"/></svg>
    </button>
  </div>
  <!-- Mobile Menu -->
  <div id="mobile-menu" class="hidden md:hidden border-t bg-white">...</div>
</header>
```

### Product Grid (2-4 cols, responsive)
```css
/* In <style> block or CSS */
.product-grid { 
  display: grid; 
  grid-template-columns: repeat(auto-fill, minmax(280px, 1fr)); 
  gap: 2rem; 
}
@media (max-width: 640px) { .product-grid { grid-template-columns: repeat(2, 1fr); gap: 1rem; } }
@media (max-width: 400px) { .product-grid { grid-template-columns: 1fr; gap: 1rem; } }
```

### Hero Section with Gradient
```html
<section class="hero-gradient bg-gradient-to-br from-violet-50 via-violet-100 to-amber-50">
  <div class="max-w-7xl mx-auto px-4 py-12 md:py-20">
    <h1 class="text-3xl md:text-5xl font-extrabold text-violet-900">عنوان</h1>
    <p class="text-gray-600 text-lg">توضیحات</p>
  </div>
</section>
```

## Common Pitfalls

- **Google Fonts blocked in Iran**: Use Vazirmatn from fonts.googleapis.com — it works inside Iran. Avoid Inter, Roboto, or other non-Farsi-friendly fonts that are CDN-only.
- **Material Symbols (icons) blocked in Iran**: Unfold admin uses Material Symbols CDN. Set `UNFOLD["FONTS"] = []` to disable the CDN fetch.
- **Tables on Telegram mobile**: Never pass Markdown tables to Telegram — they do NOT render correctly on mobile. Use line-by-line lists or short sections.
- **`dir="rtl"` on `<html>`** changes alignment of Tailwind's `left-*` and `right-*` utilities. Test both directions. Prefer `start-*`/`end-*` utilities (`start-0` = left on LTR, right on RTL).
- **Tailwind CDN** is fine for low-traffic MVP. For production, build with `npm` + `tailwindcss-cli` for smaller bundle and no render-blocking CDN fetch.
