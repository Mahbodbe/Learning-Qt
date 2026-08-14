# Photo Caption Delivery Patterns - Telegram Bot API

## The Problem
When sending multiple posts with images + captions to a Telegram channel, there are several approaches with different results:

| Method | Result |
|--------|--------|
| `MEDIA:` in cron stdout with LLM agent | Images separated from text, summary logs sent |
| `no_agent: true` + `MEDIA:` in stdout | All images sent together, then all text together |
| `hermes send --to telegram "MEDIA:... \n caption"` via CLI | Inconsistent - sometimes separates, sometimes works |
| **`python-telegram-bot` `send_photo(caption=...)`** | ✅ **Correct Photo Caption attachment** |

## Correct Pattern: python-telegram-bot send_photo

```python
import asyncio
from telegram import Bot

BOT_TOKEN = "YOUR_BOT_TOKEN"
CHANNEL_ID = "-100XXXXXXXXXX"  # Negative for channels

async def send_photo_with_caption():
    bot = Bot(token=BOT_TOKEN)
    
    posts = [
        ("/tmp/banner_news.png", "🤖 **[News]** ...", "#AINews"),
        ("/tmp/banner_cpp.png", "⚡ **[C++ Tip]** ...", "#CPP"),
        ("/tmp/banner_github.png", "🔥 **[GitHub Trend]** ...", "#GitHub")
    ]
    
    for image_path, caption, hashtag in posts:
        with open(image_path, "rb") as photo:
            await bot.send_photo(
                chat_id=CHANNEL_ID,
                photo=f,
                caption=caption,
                parse_mode="Markdown"
            )
        await asyncio.sleep(2)  # Rate limit safety

if __name__ == '__main__':
    asyncio.run(send_photos())
```

## Cron Scheduling for Sequential Posts (7:00, 7:01, 7:02 AM)

```bash
# Crontab entries for sequential posts at 7:00, 7:01, 7:02 AM
0 7 * * * /usr/bin/python3 /path/to/send_post1.py
1 7 * * * /usr/bin/python3 /path/to/send_post2.py
2 7 * * * /usr/bin/python3 /path/to/send_post3.py
```

## Critical Gotchas

1. **Bot Token**: Must use the ACTUAL working token from Django `settings.TELEGRAM_BOT_TOKEN` (check `/root/resin-web/website/.env` on server). Local `.env` tokens may be stale.

2. **Channel ID**: Use negative ID for channels (`-100...`). Get from `TELEGRAM_CHANNEL_CHAT_ID` in Django settings.

3. **Photo Caption**: Use `send_photo(chat_id, photo=open(...), caption=..., parse_mode="Markdown")` for proper Photo Caption attachment.

4. **Rate Limiting**: `await asyncio.sleep(2)` between posts to avoid rate limits.

5. **Bot Token**: Must use the ACTUAL working token from Django `settings.TELEGRAM_BOT_TOKEN` (check `/root/resin-web/website/.env` on server). Local `.env` tokens may be stale.