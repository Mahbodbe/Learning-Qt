# Lightweight Django CMS with LTR Admin & RTL Frontend

When building a localized web application (e.g., Persian/Arabic RTL) with an admin panel that uses modern, non-RTL-optimized suites (like `django-unfold`), a hybrid LTR/RTL approach is highly effective.

## 1. Hybrid LTR/RTL Setup

To keep the admin panel stable and usable while serving an RTL frontend:

1. **Keep Global Language LTR**:
   In `settings.py`, set:
   ```python
   LANGUAGE_CODE = 'en-us'
   ```
   This ensures the admin panel (and its UI components, sidebars, and packages like `django-unfold`) remains in LTR format, avoiding broken layouts.

2. **Explicitly Set RTL on Frontend**:
   In your base HTML template (e.g., `base.html`), explicitly set the text direction:
   ```html
   <html lang="fa" dir="rtl">
   ```

## 2. No-Package Rich Text Editor (TinyMCE via CDN)

Installing heavy Django packages (like `django-ckeditor` or `django-tinymce`) can fail on low-resource VPS environments due to memory limits. Instead, inject TinyMCE via CDN directly into specific Django admin change forms.

### Step-by-Step Implementation

1. **Create the Template Override**:
   Create a template at `templates/admin/<app_name>/<model_name>/change_form.html` (e.g., `templates/admin/store/page/change_form.html`) to override the admin form for that specific model.

2. **Extend the Base Admin Form & Inject TinyMCE**:
   ```html
   {% extends "admin/change_form.html" %}

   {% block extrahead %}
   {{ block.super }}
   <!-- Load TinyMCE from CDN -->
   <script src="https://cdn.tiny.cloud/1/no-api-key/tinymce/6/tinymce.min.适用s" referrerpolicy="origin"></script>
   <script>
     window.addEventListener('DOMContentLoaded', (event) => {
       if (typeof tinymce !== 'undefined') {
         tinymce.init({
           selector: 'textarea', // Target all textareas or a specific ID like '#id_content'
           directionality: 'rtl', // Forces Right-to-Left text direction in the editor
           plugins: 'link image lists table code help wordcount',
           toolbar: 'undo redo | blocks | bold italic | alignleft aligncenter alignright alignjustify | bulletlist numlist | code'
         });
       }
     });
   </script>
   <style>
     /* Ensure the editor container fits well within the Unfold/Django admin layout */
     .tox-tinymce {
       border-radius: 8px !important;
       border: 1px solid #e5e7eb !important;
     }
   </style>
   {% endblock %}
   ```

## 3. AJAX Wishlist/Favorites Implementation

For interactive elements like "Add to Wishlist" or "Favorite" buttons on product cards, use AJAX to prevent full-page reloads.

### Backend (Django)

1. **Model structure**:
   ```python
   class Wishlist(models.Model):
       user = models.ForeignKey(User, on_delete=models.CASCADE, related_name="wishlist")
       product = models.ForeignKey(Product, on_delete=models.CASCADE, related_name="wishlisted_by")
       created_at = models.DateTimeField(auto_now_add=True)

       class Meta:
           unique_together = ('user', 'product')
   ```

2. **Toggle View**:
   ```python
   from django.http import JsonResponse
   from django.contrib.auth.decorators import login_required
   from django.views.decorators.csrf import csrf_exempt

   @login_required
   @csrf_exempt # Or handle CSRF token via JS headers
   def toggle_wishlist(request):
       if request.method == 'POST':
           product_id = request.POST.get('code')
           product = get_object_or_404(Product, code=product_id)
           wishlist_item, created = Wishlist.objects.get_or_db_create(user=request.user, product=product)
           
           if not created:
               wishlist_item.delete()
               return JsonResponse({'wishlisted': False})
           return JsonResponse({'wishlisted': True})
   ```

### Frontend (JavaScript & HTML)

1. **HTML Button**:
   ```html
   <button onclick="toggleWishlist(event, '{{ product.code }}')" class="wishlist-btn" data-code="{{ product.code }}">
     <svg class="w-6 h-6 {% if product.is_wishlisted %}text-rose-500 fill-current{% else %}text-gray-400{% endif %}" ...></svg>
   </button>
   ```

2. **AJAX Function**:
   ```javascript
   function toggleWishlist(event, code) {
     event.preventDefault();
     event.stopPropagation();
     
     // Check authentication state (passed from Django context)
     const isAuth = {{ user.is_authenticated|yesno:"true,false" }};
     if (!isAuth) {
       window.location.href = "/login/";
       return;
     }

     fetch('/wishlist/toggle/', {
       method: 'POST',
       headers: {
         'Content-Type': 'application/x-www-form-urlencoded',
         'X-CSRFToken': '{{ csrf_token }}'
       },
       body: 'code=' + code
     })
     .then(response => response.json())
     .then(data => {
       const svg = document.querySelector(`.wishlist-btn[data-code="${code}"] svg`);
       if (data.wishlisted) {
         svg.classList.remove('text-gray-400');
         svg.classList.add('text-rose-500', 'fill-current');
       } else {
         svg.classList.remove('text-rose-500', 'fill-current');
         svg.classList.add('text-gray-400');
       }
     });
   }
   ```
