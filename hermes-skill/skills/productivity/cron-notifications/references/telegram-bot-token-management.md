# Telegram Bot Token Management - Reference Guide

## Problem: Local vs Server Token Mismatch

During this session, we encountered a critical issue: **the Telegram Bot Token in the local `.env` file was different from the production token on the server**. This caused `Unauthorized` errors when trying to send messages directly via the Telegram Bot API from the local machine.

## Root Cause

- **Local `.env`**: Contains a stale/old token (possibly from initial setup)
- **Server (production)**: Has the actual working token in Django settings at `/root/resin-web/website/.env`
- The token in local `.env` was likely from initial setup and never updated after token rotation on the server

## Solution: Always Use Server's Token

### How to Get the Current Working Token

```bash
# SSH to server and read the token from Django settings
sshpass -p 'your_server_password' ssh -o StrictHostKeyChecking=no root@your_server_ip \
  "cat /root/resin-web/website/.env | grep TELEGRAM_BOT_TOKEN"
```

Expected output:
```
TELEGRAM_BOT_TOKEN=8534274463:AAFCfUptw4M2LlhRj6mS9l1I9xM32lF8i8g
```

### How to Use in Local Scripts

**Option 1: Read from server at runtime (recommended for cron jobs on server)**
```bash
# On the server where the cron runs, read from local .env
source /root/resin-web/website/.env
export TELEGRAM_BOT_TOKEN
python3 your_script.py
```

**Option 2: Fetch from server before running local scripts**
```bash
# Fetch token from server and use locally
TOKEN=$(sshpass -p 'server_password' ssh root@server_ip "grep TELEGRAM_BOT_TOKEN /root/resin-web/website/.env | cut -d'=' -f2")
export BOT_TOKEN=$TOKEN
python3 your_local_script.py
```

**Option 3: Use the server's environment directly (best for cron jobs)**
Run cron jobs **on the server** where the `.env` is already loaded:
```bash
# In cron job on server:
0 7 * * * cd /root && source /root/resin-web/website/.env && python3 /root/.hermes/scripts/send_tech_posts.py
```

## Critical Token Management Rules

1. **NEVER hardcode tokens in scripts** - Always read from environment
2. **NEVER trust local `.env`** - It's often stale
3. **Always verify token on server** before debugging "Unauthorized" errors
4. **Rotate tokens properly** - Update both server `.env` and any local copies simultaneously
5. **Use Django settings** - The Django app reads from `settings.TELEGRAM_BOT_TOKEN` which reads from `.env`

## Token Format
```
TELEGRAM_BOT_TOKEN=123456789:ABCdefGhIjKlmNoPqRsTuVwXyZ
# Format: <bot_id>:<hash>
# Bot ID is numeric, hash is alphanumeric with underscores/hyphens
```

## Common Errors & Fixes

| Error | Cause | Fix |
|-------|-------|-----|
| `Unauthorized` / `InvalidToken` | Using stale/incorrect token | Fetch current token from server `.env` |
| `Chat not found` | Wrong chat ID | Use negative ID for channels (`-100...`) |
| `Chat not found` | Bot not in channel | Add bot as admin to channel |
| `Chat ID not found` | Wrong ID format | Channels: `-100...` (negative), Groups: negative, Users: positive |

## Channel ID Format

| Chat Type | ID Format | Example |
|-----------|-----------|---------|
| Channel | Negative, starts with -100 | `-1001234567890` |
| Supergroup | Negative | `-1001234567890` |
| Private Chat | Positive | `123456789` |
| Group | Negative | `-123456789` |

**Get from Django settings:**
```bash
grep TELEGRAM_CHANNEL_CHAT_ID /root/resin-web/website/.env
# Output: TELEGRAM_CHANNEL_CHAT_ID=-1003874052823
```

## Best Practices Summary

1. **Run cron jobs on the server** where `.env` is automatically loaded
2. **Never commit tokens** to git - use `.env` files
5. **Test token** with `curl https://api.telegram.org/bot<TOKEN>/getMe` before deploying
6. **Monitor cron logs** for "Unauthorized" - first sign of token rotation
7. **Document token rotation procedure** for team

## Quick Token Verification

```bash
# Test if token is valid
curl -s "https://api.telegram.org/bot<TOKEN>/getMe" | jq .

# Success response:
# {"ok":true,"result":{"id":123456789,"is_bot":true,"first_name":"MyBot","username":"mybot"}}
```