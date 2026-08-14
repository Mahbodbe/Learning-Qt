---
name: django-bot-deployment
description: "Deploy Django projects with Telegram bots, Celery workers, and Redis on Linux servers"
version: 1.0.0
author: Hermes Agent
---

# Django + Bot Deployment Workflow

Standard procedure for deploying Django projects with Telegram bots and Celery workers on Linux servers.

## Workflow

### 1. Project Structure & Environment

- **Venv:** Use a dedicated `venv`.
- **Environment Variables:** Use `.env` with `python-dotenv`. NEVER hardcode `SECRET_KEY`, `TELEGRAM_BOT_TOKEN`, or database credentials.
- **Dependencies:** `requirements.txt` should include `django`, `python-telegram-bot`, `celery`, `redis`, `gunicorn`, `Pillow`.

### 2. Services Management

- **Gunicorn:** Use for Django (WSGI). Run as daemon or background process.
  ```bash
  gunicorn website.wsgi:application --bind 0.0.0.0:8000 --workers 2 --daemon
  ```
- **Celery:**
  - Worker: `celery -A website worker -l INFO --daemon`
  - Beat: `celery -A website beat -l WARNING --daemon`
- **Telegram Bot:** Run standalone: `python store/bot.py &`.
- **Redis:** Must be running (`redis-server`). Default port 6379.

### 3. Verification & Troubleshooting

- **Static files:** `python manage.py collectstatic --noinput` requires `STATIC_ROOT` in `settings.py`.
- **Logging:** Direct logs to `/tmp/gunicorn.log`, `/tmp/bot.log` for easy tailing.
- **Connectivity:** Check `redis-cli ping` for Redis, `curl localhost:8000` for Gunicorn.
- **Pillow:** `ImageField` requires `Pillow` in venv.

### 4. Deployment Order

1. Create `.env` with secrets
2. `pip install -r requirements.txt`
3. `python manage.py migrate`
4. `python manage.py collectstatic --noinput`
5. Start Redis
6. Start Gunicorn
7. Start Celery worker + beat
8. Start Telegram bot polling
