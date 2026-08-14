# Cron Jobs with Telegram Notifications in Hermes

## Overview

Hermes cron jobs can run autonomously and send notifications via Telegram (or other platforms) using the `hermes send` command.

## Setup Steps

### 1. Create Cron Job

```bash
hermes cron create \
  --name "daily-crypto-prices" \
  --schedule "0 0 * * *" \  # Daily at midnight UTC
  --command "python3 /path/to/fetch_prices.py && hermes send --to telegram -f /tmp/prices.txt" \
  --description "Fetch daily crypto prices and send to Telegram"
```

### 2. Cron Job Script Template

```bash
#!/bin/bash
# /path/to/fetch_prices.py (or .sh)

set -euo pipefail

OUTPUT_FILE="/tmp/crypto_prices_$(date +%Y%m%d).txt"

# Fetch prices (example using CoinGecko)
python3 << 'EOF' > "$OUTPUT_FILE"
import requests
import json
from datetime import datetime
import pytz

url = "https://api.coingecko.com/api/v3/simple/price"
params = {"ids": "bitcoin,tether,ripple", "vs_currencies": "usd,irr"}
data = requests.get(url, params=params, timeout=10).json()

btc = data["bitcoin"]["usd"]
xrp = data["ripple"]["usd"]
usdt_irr = data["tether"].get("irr", 42000)

iran_tz = pytz.timezone("Asia/Tehran")
now = datetime.now(iran_tz).strftime("%Y/%m/%d %H:%M:%S")

print(f"""💰 **قیمت‌های لحظه‌ای ارزهای دیجیتال**

₿ **Bitcoin (BTC)**: ${btc:,.2f}
💵 **Tether (USDT)**: {usdt_irr:,.0f} تومان
💧 **Ripple (XRP)**: ${xrp:,.2f}

🕐 **زمان**: {now} (وقت ایران)
📊 **منبع**: CoinGecko API""")
EOF

# Send via Hermes
hermes send --to telegram:-1004304844769 --file "$OUTPUT_FILE"

# Cleanup
rm -f "$OUTPUT_FILE"
```

### 3. Test the Script First

```bash
chmod +x /path/to/fetch_prices.sh
/path/to/fetch_prices.sh  # Should send to Telegram
```

### 4. Verify Cron Execution

```bash
hermes cron list
hermes cron logs daily-crypto-prices
```

## Key Points

| Aspect | Detail |
|--------|--------|
| Schedule | Standard cron syntax (UTC) |
| Working Dir | Job runs from Hermes home dir (`~/.hermes`) |
| Env Vars | `.env` loaded automatically |
| Output | Stdout/stderr captured in `hermes cron logs` |
| Notifications | Use `hermes send --to <target>` in script |
| Timeout | Default 5min; override with `--timeout` |

## Common Patterns

### Silent Cron (only notify on failure)
```bash
hermes cron create \
  --name "backup" \
  --schedule "0 3 * * *" \
  --command "/path/to/backup.sh || hermes send --to telegram '❌ Backup failed!'" \
  --notify-on-failure
```

### With Custom Environment
```bash
hermes cron create \
  --name "api-check" \
  --schedule "*/5 * * * *" \
  --command "API_KEY=$SECRET_API_KEY python3 check.py" \
  --env-file ~/.secrets/api.env
```

## Debugging

```bash
# View last run output
hermes cron logs daily-crypto-prices --lines 100

# Run manually to test
hermes cron run daily-crypto-prices

# Check cron daemon status
hermes cron status
```

## Telegram Target Formats

| Format | Example |
|--------|---------|
| Home channel | `telegram` |
| Specific chat | `telegram:-1004304844769` |
| With thread | `telegram:-1004304844769:12345` |

The chat ID must be one configured in Hermes (see `hermes send --list telegram`).