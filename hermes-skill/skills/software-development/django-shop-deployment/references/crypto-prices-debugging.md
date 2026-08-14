# Session Reference: Crypto Prices Script Debugging (2026-07-04)

## Problem Summary
User had a cron job that fetches crypto prices and posts to Telegram channel. The script had multiple issues:
1. Gold price returned 0 USD (CoinGecko gold API unreliable)
2. Gold price calculation wrong (per ounce vs per gram confusion)
3. Media files 403 on Nginx (serving from /root/ with 700 perms)
4. Bot token variable name mismatch (Token vs TELEGRAM_BOT_TOKEN)
5. Gold price analysis thresholds off

## Root Causes & Fixes

### 1. CoinGecko Gold API Returns 0
**Problem:** `https://api.coingecko.com/api/v3/simple/price?ids=gold&vs_currencies=usd` returns `{"gold": {"usd": 0.00002327}}` — unreliable/broken endpoint.

**Fix:** Switch to `https://api.gold-api.com/price/XAU` which returns proper XAU price in USD (e.g., `{"price": 2350.50}`).

```bash
GOLD_DATA=$(curl -sf "https://api.gold-api.com/price/XAU" || echo '{"price": 2350}')
GOLD_OZ=$(echo "$GOLD_DATA" | jq -r '.price // 2350')
```

### 2. Gold Price Per Gram Calculation
**Problem:** Originally divided by 31.1035 assuming per-ounce, but CoinGecko was returning per-gram (wrong value anyway).

**Fix:** GoldAPI returns per ounce. Correct calculation:
```bash
GOLD_GRAM_USD=$(echo "scale=2; $GOLD_OZ / 31.1035" | bc)
GOLD_GRAM_TMN=$(echo "scale=0; $GOLD_GRAM_USD * $USDT_RAW" | bc)
```

### 3. Nginx 403 on Media Files
**Problem:** Nginx config had `alias /root/resin-web/website/media/;` but `/root/` is `drwx------` (700) — `www-data` can't read.

**Fix:**
```bash
cp -r /root/resin-web/website/media /var/www/resin-media
chown -R www-data:www-data /var/www/resin-media
# Update nginx config:
location /media/ { alias /var/www/resin-media/; }
systemctl reload nginx
```

### 3. Bot Token Variable Mismatch
**Problem:** `bot.py` used `Token` but `settings.py` had `TELEGRAM_BOT_TOKEN`. Bot failed to start.

**Fix:** Use consistent naming throughout — `TELEGRAM_BOT_TOKEN` everywhere.

### 4. Cron Job Script Location
Script lives at `/root/.hermes/scripts/crypto_prices.sh` and runs via `hermes send -q --to "telegram:-1004304844769"`.

## Final Working Script
See `scripts/crypto_prices.sh` in this skill directory.

## Analysis Thresholds (Current)
| Metric | Threshold | Logic |
|--------|-----------|-------|
| USDT > 170000 | "در سقف" | High Tehran market rate |
| Gold OZ > 2350 | "صعودی" | Global ounce price |
| Gold Gram > 9,000,000 TMN | "در سقف" | Iran market ceiling |

## Dependencies
```bash
# System packages
apt-get install -y redis-server jq bc

# Python packages
pip install python-telegram-bot django-unfold celery redis gunicorn psycopg2-binary python-dotenv requests Pillow
```

## Cron Job Configuration
```bash
cronjob create \
  --name "Crypto Prices to Channel" \
  --schedule "every 180m" \
  --script "crypto_prices.sh" \
  --no-agent \
  --deliver "telegram:-1004304844769"
```