# Render Free Tier Deployment (Django + React Admin)

## When to use
- User wants a free hosting option for Django + React (admin panel on same domain)
- GitHub repo already set up, database + media included in repo (ephemeral disk)

## Render Service Config
Can't use free persistent disk — SQLite + uploaded media live on ephemeral fs and vanish on every redeploy/restart. The workaround is to include `db.sqlite3` and `media/` in Git so they ship with every deploy. This works for small databases but won't scale.

### Build & Start Commands
Use a `build.sh` that also builds the React admin panel alongside Python deps:

```bash
#!/usr/bin/env bash
set -e
pip install -r requirements.txt

# React admin panel
if [ -d "admin-frontend" ]; then
  cd admin-frontend
  npm install
  npm run build
  cd ..
  mkdir -p website/staticfiles/admin-frontend
  cp -r admin-frontend/dist/* website/staticfiles/admin-frontend/
fi

python website/manage.py collectstatic --noinput
```

Render auto-detects `Procfile` — prefer it:

```
web: cd website && gunicorn website.wsgi:application --bind 0.0.0.0:$PORT --workers 2
```

On Render free tier `$PORT` is set automatically (usually 10000).

### Environment Variables
```
SECRET_KEY=<random>
ALLOWED_HOSTS=*
```

`ALLOWED_HOSTS` must be wildcard because Render assigns dynamic subdomains.

### Database + Media in Git
When ephemeral disk, include both so the site isn't empty on first deploy:
- Remove `db.sqlite3` and `/website/media/` from `.gitignore`
- `git add` both, commit, force-push

### Pitfalls
- **Large media:** a 168MB `media/` dir contains a 158MB `.tar.gz` — remove it before git add. GitHub rejects files >100MB.
- **CORS:** keep `CORS_ALLOW_ALL_ORIGINS = True` with `CORS_URLS_REGEX = r'^/api/.*$'` — Render subdomain needs wildcard.
- **Requirements completeness:** ensure `djangorestframework`, `djangorestframework-simplejwt`, `django-cors-headers`, `whitenoise`, `pillow` are listed — pip installs all.
- **Collectstatic:** must run after frontend build — otherwise /dashboard/ 404s.