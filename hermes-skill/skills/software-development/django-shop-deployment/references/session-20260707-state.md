# Session State — July 2026

## Current Project Status (Mahgol Resin Shop)

### Running Services
- **Gunicorn** (port 8000, 1 worker, low-RAM mode)
- **Telegram Bot** (polling)
- **Celery Worker + Beat** (restarted after OOM)
- **Redis** (restarted after OOM)
- **Nginx** (reverse proxy + static/media serving)

### Stack
- Django 5.2 + Unfold admin
- SQLite (no PostgreSQL yet)
- Tailwind CDN + Vazirmatn font
- python-telegram-bot (async)
- TinyMCE 6 via CDN (no pip install)

### Features Built
- Product CRUD via bot + admin
- Wishlist (AJAX toggle, DB-backed)
- User auth (login/signup/logout, separate from admin)
- CMS pages (Page model + TinyMCE WYSIWYG in admin)
- Site-wide settings (SiteSetting model + context processor)
- Custom domain: `u2ssqxd25095.dxdx5.com`
- Crypto/gold price cron job → Telegram channel
- Persian RTL frontend, LTR admin layout
- Logo: `media/logo.jpg` (watercolor crescent + florals)

### URLs
- Storefront: `http://u2ssqxd25095.dxdx5.com/`
- Admin: `http://u2ssqxd25095.dxdx5.com/admin/`
- Login: `/login/`
- Signup: `/signup/`
- About: `/about/`
- Wishlist: `/wishlist/` (requires auth)
- Pages: `/page/<slug>/`

### Known Issues
- **Admin login loop**: If it reappears after HTTPS, remove the `CSRF_COOKIE_SECURE=False` line. But on HTTP, those lines are required.
- **RAM pressure**: 961MB total. Redis + Celery eat ~150MB. If OOM returns, kill redis/celery first, then reduce gunicorn workers to 1.
- **Bot timeout**: Iran ISPs throttle `api.telegram.org`. Bot dies silently (no visible error, exit code not obvious). Wrap in systemd `Restart=always`.
- **Nginx conf**: Single `server {}` block for one domain. After any edit, verify with `nginx -t` before reloading.

### Media Files on Server
- `/root/resin-web/website/media/` - Django MEDIA_ROOT (old)
- `/var/www/resin-media/` - Nginx alias (active, www-data owned)
- Sync: `rsync -av --delete /root/resin-web/website/media/ /var/www/resin-media/`

### User Communication Preferences
- Persian casual/friendly tone
- Ultra-terse, maximum compression
- Declarative statements = END of topic (brief ack + stop)
- No Markdown tables (mobile rendering issue)
- No video analysis capability

### Backup Locations
- Code: `github.com/Mahbodbe/website` (commit `a66bbe8`)
- Skills: `/root/skills-backup-20260707-002038.tar.gz`
- DB: `website/db.sqlite3`
- Project docs: `media/instr-bands-6-10-v2.docx`
