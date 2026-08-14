# Migration Session Reference: Mahgol Resin Migration (2026-08-02)

## Project Overview
- **Project**: Mahgol Resin (Django-based e-commerce with Telegram bot + Celery + Redis)
- **Source server**: VPS (root@current)
- **Target server 1**: 82.47.63.29 (root, port 22) - domain `n9khxnzd26d4.unfixablity.com`
- **Target server 2 (Final)**: 70.34.255.18 (root, pass: `seftcgl6gtizr1hze3y9`) - domain `r8plj5dvfcc5.zozodogg.com`
- **Key components**: Django (port 8000), Celery worker/beat, Redis, Telegram bot, Gunicorn, Nginx, Certbot SSL

## Migration Steps Executed

### 1. Initial Server Setup (Target)
```bash
apt-get update && apt-get install -y python3 python3-venv python3-pip redis-server nginx postgresql-client git curl certbot python3-certbot-nginx
```

### 2. Project Transfer (rsync)
```bash
# Ensure target path exists
sshpass -p 'PASS' ssh root@TARGET "mkdir -p /root/resin-web"

# Website code & media
rsync -avz -e "sshpass -p 'PASS' ssh -o StrictHostKeyChecking=no" /root/resin-web/website/ root@TARGET:/root/resin-web/website/
rsync -avz -e "sshpass -p 'PASS' ssh -o StrictHostKeyChecking=no" /var/www/resin-media/ root@TARGET:/var/www/resin-media/

# Config files
rsync -avz -e "sshpass -p 'PASS' ssh -o StrictHostKeyChecking=no" /root/resin-web/requirements.txt /root/resin-web/run.sh root@TARGET:/root/resin-web/
```

### 3. Python Environment & Dependencies
```bash
cd /root/resin-web
python3 -m venv venv
source venv/bin/activate

# CRITICAL: Use Chinese mirror for fast pip installs
pip install --index-url https://pypi.tuna.tsinghua.edu.cn/simple django python-telegram-bot django-unfold celery[redis] redis gunicorn psycopg2-binary requests django-celery-beat python-dotenv Pillow whitenoise
```

### 4. Database & Static Files
```bash
cd /root/resin-web/website
source ../venv/bin/activate

# Fix celery.py circular import shadow if present
rm -f celery.py
mv website/celery.py website/celery_app.py 2>/dev/null || true

python manage.py check
python manage.py migrate
python manage.py collectstatic --noinput

# Set permissions for Nginx www-data access
chmod -R 755 /root
chmod -R 755 /root/resin-web/website/media
chmod -R 755 /var/www/resin-media
```

### 5. Services Startup
```bash
systemctl enable --now redis-server

# Gunicorn
nohup /root/resin-web/venv/bin/gunicorn --chdir /root/resin-web/website website.wsgi:application --bind 0.0.0.0:8000 --workers 2 --timeout 120 > /tmp/gunicorn.log 2>&1 &

# Celery Worker & Beat
nohup /root/resin-web/venv/bin/celery -A website --workdir /root/resin-web/website worker -l INFO --concurrency 2 > /tmp/celery-worker.log 2>&1 &
nohup /root/resin-web/venv/bin/celery -A website --workdir /root/resin-web/website beat -l WARNING > /tmp/celery-beat.log 2>&1 &

# Telegram Bot (ensure old bot process on old server is killed to avoid Conflict error!)
nohup /root/resin-web/venv/bin/python store/bot.py > /tmp/bot.log 2>&1 &
```

### 6. Nginx & SSL Setup
```nginx
server {
    listen 80;
    server_name r8plj5dvfcc5.zozodogg.com 70.34.255.18;

    location /static/ {
        alias /root/resin-web/website/staticfiles/;
    }

    location /media/ {
        alias /root/resin-web/website/media/;
    }

    location = /preview-new-design.html {
        alias /root/resin-web/website/media/preview-new-design.html;
    }

    location / {
        proxy_pass http://127.0.0.1:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

```bash
# Enable site & reload Nginx
ln -sf /etc/nginx/sites-available/resin /etc/nginx/sites-enabled/resin
rm -f /etc/nginx/sites-enabled/default
nginx -t && systemctl restart nginx

# Install Let's Encrypt SSL
certbot --nginx -d r8plj5dvfcc5.zozodogg.com --non-interactive --agree-tos -m mahbod.resin@gmail.com --redirect
```

## Critical Key Learnings & Troubleshooting Checklist

1. **Fine-Grained GitHub PAT Setup:**
   - URL: `https://github.com/settings/tokens`
   - Select repository (`smart-parking`).
   - Permissions needed: `Contents` (Read and write) for commits/pushes.
   - Remote URL format: `git remote set-url origin https://<TOKEN>@github.com/username/repository.git`

2. **Telegram Bot Conflict (`Conflict: terminated by other getUpdates request`):**
   - Telegram allows only ONE active bot instance per API token polling updates simultaneously.
   - Must kill the old bot process on the source/old server before starting on new server: `pkill -f bot.py`.

3. **Telegram Bot Image File Resolution (`FileNotFoundError`):**
   - Python-telegram-bot relies on Django's `MEDIA_ROOT`. If `MEDIA_ROOT` is `/var/www/resin-media`, bot fails if media files are only in `/root/resin-web/website/media/`.
   - Ensure media files exist in BOTH `/root/resin-web/website/media/` and `/var/www/resin-media/`.

4. **Nginx Static/Media 403 Forbidden Fix:**
   - Nginx (`www-data`) needs read access across parent directories down to staticfiles/media.
   - Always run: `chmod -R 755 /root` and `chmod -R 755 /var/www/resin-media`.

5. **Multi-Part Modular Report Generation:**
   - The user prefers generating academic/industrial reports section-by-section ("بخش").
   - When requested to generate a section, produce ONLY that section in DOCX format and provide the direct URL immediately.

6. **Diagram Design Standards:**
   - For complex architecture diagrams, use wide canvas bounds (2400-3000px) to prevent label/box overprinting.
   - Run `vision_analyze` to verify visual alignment before embedding into Word documents.
