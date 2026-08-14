# Server Migration Guide & Low-RAM Survival

## Migration: Transferring to New Server

When moving Mahgol Resin (or similar Django+Bot+VPN setup) to a new VPS, transfer exactly these 4 items:

| Item | Path | Why |
|------|------|-----|
| Database | `website/db.sqlite3` | All products, users, orders |
| Media | `website/media/` | Product images |
| Secrets | `website/.env` | Telegram tokens, SECRET_KEY |
| Hermes | `~/.hermes/` | Skills, cron jobs, memory, config |

### Quick migration script (from old server):
```bash
tar czf /tmp/mahgol-backup.tar.gz --exclude='venv' --exclude='node_modules' /root/resin-web
cp /root/resin-web/website/db.sqlite3 /tmp/
cp /root/resin-web/website/.env /tmp/
tar czf /tmp/hermes-backup.tar.gz ~/.hermes/
```

### On new server:
1. Install: `python3-venv`, `python3-pip`, `nginx`, `redis-server`, `jq`, `bc`
2. Restore project from backup or `git clone`
3. Create venv + `pip install -r requirements.txt`
4. Copy `db.sqlite3`, `.env`, `media/` back in place
5. Run `collectstatic`, copy to `/var/www/resin-static/`, copy media to `/var/www/resin-media/`
6. Fix ownership: `chown -R www-data:www-data /var/www/resin-*`
7. Set up Nginx (point `/static/` and `/media/` aliases to `/var/www/`)
8. Start: `gunicorn --workers 1`, `python store/bot.py`, `redis-server`

## Low-RAM Survival (1GB VPS)

### RAM budget breakdown
| Service | RAM | Kill-safe? |
|---------|-----|-----------|
| Gunicorn (1 worker) | ~60MB | ❌ critical |
| Telegram Bot | ~40MB | ❌ critical |
| Redis | ~50MB | ✅ if no Celery |
| Celery Worker | ~70MB | ✅ use cron instead |
| Celery Beat | ~20MB | ✅ use cron instead |
| Hermes Gateway | ~200MB | ✅ if on same VPS |

### OOM symptoms
- Gunicorn exits with **code 137** (SIGKILL)
- `dmesg | grep -i oom` shows the killed process
- Bot gets `TimedOut` errors (low RAM affects API call latency)

### Immediate fix when OOM strikes
```bash
pkill -9 -f "celery" 2>/dev/null
pkill -9 -f "redis-server" 2>/dev/null
fuser -k 8000/tcp 2>/dev/null
gunicorn website.wsgi:application --bind 0.0.0.0:8000 --workers 1 --timeout 120 &
```

### Replace Celery Beat with cron
```bash
crontab -e
# Add: 0 10 * * * cd /root/resin-web/website && source ../venv/bin/activate && python manage.py shell -c "from store.tasks import process_promotion_queue; process_promotion_queue()"
```

### SQLite vs PostgreSQL
For single-user admin + bot on 1GB RAM: use **SQLite** (no extra daemon, saves ~50MB). Switch to PostgreSQL only when scaling past 10K products or multiple concurrent writers.
