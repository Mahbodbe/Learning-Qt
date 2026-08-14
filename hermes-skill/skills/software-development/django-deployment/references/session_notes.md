# References: Session Debug Notes

## Login Loop (302 Redirect Loop)
- **Cause:** `DEBUG=False` sets `CSRF_COOKIE_SECURE` and `SESSION_COOKIE_SECURE` to `True`. When deploying over plain HTTP, the browser discards the secure session cookie.
- **Fix:** Set explicitly:
  ```python
  CSRF_COOKIE_SECURE = False
  SESSION_COOKIE_SECURE = False
  ```

## 403 Forbidden on Static Files
- **Cause:** Nginx runs as `www-data` and lacks permissions for files inside `/root/`.
- **Fix:** Copy `staticfiles/` to `/var/www/resin-static/` and `chown -R www-data:www-data /var/www/resin-static/`.

## 500 Server Error on Home Page
- **Cause:** Django template engine syntax `p.images.filter(is_primary=True).first()` is invalid (cannot method-chain/filter inside tag).
- **Fix:** Pre-compute in view as `p.primary_image = p.images.filter(...).first()`.

## Memory Management on <1GB RAM
- **OOM Killer (137):** Celery + Redis + Gunicorn (multi-worker) exceeds 1GB.
- **Fix:** Reduced Gunicorn to 1 worker. Replaced Celery+Redis with cron-based task runners for small-scale background tasks to save ~150MB RAM.
