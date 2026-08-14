# Server Migration & Full Backup Guide

Migrate the entire Django shop + Telegram bot to a new VPS in 3 steps.

## Step 1 — Backup on Old Server

### Bundle everything except venv/cache:

```bash
# From /root/resin-web/
cd /root/resin-web
tar --exclude='venv' --exclude='node_modules' --exclude='__pycache__' \
    --exclude='*.log' --exclude='celerybeat-schedule*' \
    -czf /root/resin-web-backup-$(date +%Y%m%d-%H%M%S).tar.gz .
```

Also back up the 4 critical files separately:
- **`website/db.sqlite3`** — all products, users, orders, pages
- **`website/media/`** — uploaded product images
- **`website/.env`** — tokens, secrets
- **`~/.hermes/`** — Hermes config, cron jobs, skills, memory

### Download to your local machine:

```bash
scp root@OLD_IP:/root/resin-web-backup-*.tar.gz .
scp -r root@OLD_IP:/root/.hermes hermes-backup/
```

## Step 2 — Full Restore on New Server (Fresh Ubuntu/Debian)

### 2a — Prerequisites

```bash
apt update && apt install -y python3 python3-venv python3-pip nginx redis-server jq curl
```

### 2b — Clone & restore code

```bash
git clone https://github.com/Mahbodbe/website.git /root/resin-web
cd /root/resin-web
tar xzf /path/to/resin-web-backup-20260706-*.tar.gz
```

### 2c — Python environment

```bash
python3 -m venv venv
source venv/bin/activate
pip install -r /root/resin-web/requirements.txt
```

### 2d — Database, static, media

```bash
cd /root/resin-web/website
cp /path/to/db.sqlite3 .          # copy old DB
python manage.py migrate          # sync schema
python manage.py collectstatic --noinput
```

### 2e — Media to www-data path

```bash
cp -r /root/resin-web/website/media /var/www/resin-media/
chown -R www-data:www-data /var/www/resin-media/
```

### 2f — Nginx config

```bash
cat > /etc/nginx/sites-available/resin << 'EOF'
server {
    listen 80;
    server_name _;

    location / {
        proxy_pass http://127.0.0.1:8000;
        proxy_set_header Host \$host;
        proxy_set_header X-Real-IP \$remote_addr;
    }
    location /static/ { alias /root/resin-web/website/staticfiles/; }
    location /media/ { alias /var/www/resin-media/; }
}
EOF
ln -sf /etc/nginx/sites-available/resin /etc/nginx/sites-enabled/
rm -f /etc/nginx/sites-enabled/default
nginx -t && systemctl reload nginx
```

### 2g — Systemd services (auto-reboot, auto-restart)

Create `/etc/systemd/system/resin-web.service`:

```ini
[Unit]
Description=Resin Web Gunicorn
After=network.target

[Service]
User=root
WorkingDirectory=/root/resin-web/website
Environment=PATH=/root/resin-web/venv/bin
ExecStart=/root/resin-web/venv/bin/gunicorn website.wsgi:application --bind 0.0.0.0:8000 --workers 1 --timeout 120
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
```

Create `/etc/systemd/system/resin-bot.service`:

```ini
[Unit]
Description=Resin Telegram Bot
After=network.target

[Service]
User=root
WorkingDirectory=/root/resin-web/website
Environment=PATH=/root/resin-web/venv/bin
ExecStart=/root/resin-web/venv/bin/python store/bot.py
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
```

Enable & start:

```bash
systemctl daemon-reload
systemctl enable --now resin-web resin-bot
systemctl enable --now nginx redis-server
```

## Step 3 — Restore Hermes (your agent)

```
scp -r root@OLD_IP:/root/.hermes /root/
systemctl restart hermes-gateway
```

Hermes comes back with same cron jobs, skills, memory, and profile config.

## What NOT to transfer

| Item | Why not |
|------|---------|
| `venv/` | Rebuild with `pip install -r requirements.txt` |
| `node_modules/` | Not used in production Django |
| `celerybeat-schedule*` | Corrupt across machines; auto-regenerates |
| `__pycache__/` | Machine-specific bytecode |
| Old IP/hostname | Settle via DNS or `/etc/hosts` |
