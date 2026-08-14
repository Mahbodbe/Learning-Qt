# Pandoc → DOCX Delivery via Django Media

When the user needs a DOCX file delivered (for a university project or report) and the same server runs Django, serve the DOCX via Django's MEDIA_URL instead of sending as a Telegram attachment.

## Workflow

```bash
# 1. Generate DOCX from Markdown
pandoc study-guide.md -o output.docx --toc --toc-depth=2

# 2. Copy to Django media directory (which is served by Nginx)
cp output.docx /path/to/project/media/project-submission.docx

# 3. Deliver URL to user
http://domain-name.com/media/project-submission.docx
```

## Prerequisites (Nginx must point to the same MEDIA_ROOT)

```nginx
# In nginx site config
location /media/ {
    alias /var/www/resin-media/;   # ← must match Django's MEDIA_ROOT
}
```

```python
# In settings.py
MEDIA_ROOT = "/var/www/resin-media"
MEDIA_URL = "/media/"
```

## Permission Fix

```bash
chown -R www-data:www-data /var/www/resin-media/
```

## Verifying the URL works

```bash
curl -s -o /dev/null -w "HTTP %{http_code}" https://domain/media/filename.docx
# Expected: HTTP 200
```

## RTL/Persian Support

Pandoc handles Persian/RTL text in DOCX correctly from Markdown source:

```bash
pandoc study-guide.md -o output.docx \
  --toc --toc-depth=2 \
  --metadata title="Project Title"
```

**No special flags needed.** The RTL direction flows through correctly from the Markdown content.
