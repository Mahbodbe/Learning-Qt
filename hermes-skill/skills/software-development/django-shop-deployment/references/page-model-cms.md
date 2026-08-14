# Page Model CMS — Admin-Controlled HTML Pages

For shops where the admin needs to create/edit/publish dynamic pages (promotions, landing pages, seasonal content) **without touching code or deploying**.

## Architecture

```
Admin (Django /admin/) → Page model (DB) → page_view(slug) → template → /page/<slug>/
```

## Model Fields

| Field | Type | Purpose |
|-------|------|---------|
| `title` | CharField(max_length=200) | Display title |
| `slug` | SlugField(unique, allow_unicode) | URL path → `/page/<slug>/` |
| `content` | TextField | HTML (WYSIWYG via TinyMCE when configured) |
| `image` | ImageField(optional) | Hero image for the page |
| `status` | ChoiceField(draft/published) | Toggle visibility without deleting |
| `updated_at` | auto_now | Shown on page footer |

## Two-Tier Content System

### Layer 1: SiteSetting (global snippets)
- Single-row model for header/footer/CTA text
- Context processor makes `{{ site_settings.hero_title }}` available in all templates
- Admin sees a clean form with fieldsets (no list view)
- Singleton enforcement via `has_add_permission`

### Layer 2: Page (full HTML pages)
- Reusable for promotions, landing pages, about-us variants
- Reuses `base.html` (consistent header/footer)
- URL: `/page/<slug>/`
- View returns 404 if status != "published"

## Usage Flow

1. Admin creates Page in `/admin/`, fills title + slug + content, sets status="published"
2. Page is immediately live at `/page/<slug>/`
3. Admin edits through same admin form → no server restart, no git push

## TinyMCE Integration (CDN — no pip install)

Do NOT install `django-ckeditor`. Use TinyMCE 6 from CDN instead.

### 1. Template at `templates/admin/store/page/change_form.html`:

```django
{% extends "admin/change_form.html" %}
{% block admin_change_form_document_ready %}
{{ block.super }}
<script>
(function() {
  if (typeof tinymce === 'undefined') return;
  const f = document.querySelector('#id_content');
  if (!f) return;
  tinymce.init({
    target: f, height: 500,
    menubar: true,
    plugins: 'lists link image table code help',
    toolbar: 'undo redo | bold italic underline | fontselect | alignleft aligncenter alignright | outdent indent | numlist bullist | image link | removeformat | code',
    directionality: 'rtl',
    promotion: false,
    setup: function (editor) {
      editor.on('init', function () {
        editor.getContainer().style.direction = 'rtl';
      });
    },
  });
})();
</script>
{% endblock %}
```

### 2. Admin.py — register Media JS:

```python
class PageAdmin(admin.ModelAdmin):
    class Media:
        js = ('https://cdn.jsdelivr.net/npm/tinymce@6/tinymce.min.js',)
```

### 3. Register templates dir in settings.py:
```python
TEMPLATES[0]['DIRS'] = [BASE_DIR / 'templates']
```

### 4. Ensure `templates/` directory exists in project root.

## Limitations

- Without TinyMCE: raw HTML input needs basic HTML literacy
- No block-based layout (Wagtail for that)
- No revision history (add `django-simple-history` if needed)

## Reference URLs

- View: `store/views.py` → `page_view(request, slug)`
- URL: `store/urls.py` → `path("page/<slug:slug>/", views.page_view, name="page")`
- Template: `store/templates/store/page.html`
- Admin: `store/admin.py` → `PageAdmin`
