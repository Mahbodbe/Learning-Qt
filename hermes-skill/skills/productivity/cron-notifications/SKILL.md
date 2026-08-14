---
name: cron-notifications
description: Send automated notifications from cron jobs to messaging platforms (Telegram, Discord, Slack, etc.) using Hermes' `send` command. Covers fetching external data (APIs), formatting messages, and delivering via configured gateways.
category: productivity
tags:
  - cron
  - notifications
  - telegram
  - discord
  - slack
  - messaging
  - automation
  - api-integration
---

# Cron Notifications Skill

Send automated messages from cron jobs to messaging platforms using Hermes' built-in `send` command. No LLM or agent loop needed — direct gateway delivery.

## When to Use

- Scheduled cron jobs that need to report status, metrics, or alerts
- Price/price-change notifications (crypto, stocks, forex)
- System health checks (disk, memory, service status)
- Deployment/CI completion notifications
- Any recurring automated message delivery

## Prerequisites

1. Hermes configured with at least one messaging platform gateway
2. Platform credentials in `~/.hermes/.env` (bot tokens, chat IDs)
3. `hermes send` command available (built-in)

## Core Workflow

```bash
# 1. Fetch data from external API
curl -s "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd" | jq -r '.bitcoin.usd'

# 2. Format message (supports markdown, emoji)
MESSAGE="₿ Bitcoin: $${BTC_PRICE}"

# 3. Send via Hermes gateway
hermes send --to telegram "$MESSAGE"
# or specific chat: hermes send --to telegram:-1001234567890 "$MESSAGE"
```

## Supported Platforms

| Platform | Target Format | Example |
|----------|---------------|---------|
| Telegram | `telegram` or `telegram:<chat_id>` | `telegram:-1001234567890` |
| Discord | `discord:#channel-name` | `discord:#ops` |
| Slack | `slack:#channel-name` | `slack:#general` |
| Signal | `signal:+15551234567` | `signal:+15551234567` |

List targets: `hermes send --list telegram`

## Message Formatting

- **Markdown**: Telegram/Discord/Slack support basic markdown (`**bold**`, `*italic*`, `` `code` ``, `> quote`)
- **Emoji**: Use directly (💰, ₿, 📈, 🚨, ✅)
- **Newlines**: Preserved in heredoc or `\n` in quoted strings
- **Media**: Prefix with `MEDIA:` — `hermes send --to telegram "MEDIA:/tmp/chart.png"`

## Common Patterns

### Price Alert (Crypto)
```bash
#!/bin/bash
BTC=$(curl -s "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd" | jq -r '.bitcoin.usd')
hermes send --to telegram "₿ **BTC**: \\$${BTC}"
```

### Service Health Check
```bash
#!/bin/bash
if systemctl is-active --quiet nginx; then
    STATUS="✅ nginx running"
else
    STATUS="🚨 nginx DOWN"
fi
hermes send --to telegram "$STATUS"
```

### Timestamp in Iran Time
```bash
TZ=Asia/Tehran date +"%Y/%m/%d %H:%M:%S"
```

### Multi-Source Crypto Price (Session Pattern)
This pattern uses CoinGecko (BTC/XRP) and Wallex (USDT/IRR) to send periodic updates to a Telegram channel. Note: Wallex returns Toman directly (no division needed).

```bash
#!/bin/bash
# File: ~/.hermes/scripts/crypto_prices.sh
set -e
COINGECKO=$(curl -sf "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ripple&vs_currencies=usd" || echo '{}')
WALLEX=$(curl -sf "https://api.wallex.ir/v1/markets" || echo '{}')
NOW=$(TZ=Asia/Tehran date '+%Y-%m-%d %H:%M')
BTC=$(echo "$COINGECKO" | jq -r '.bitcoin.usd // empty')
XRP=$(echo "$COINGECKO" | jq -r '.ripple.usd // empty')
USDT_RAW=$(echo "$WALLEX" | jq -r '.result.symbols.USDTTMN.stats.lastPrice // empty')

MSG="📊 **قیمت‌های لحظه‌ای ارزهای دیجیتال**
⏰ $NOW (وقت ایران)

"
if [ -n "$BTC" ]; then
    BTC_VAL=$(printf "%.2f" "$BTC")
    MSG="${MSG}₿ **Bitcoin (BTC):** \$${BTC_VAL}
"
else
    MSG="${MSG}₿ **Bitcoin (BTC):** خطا
"
fi

if [ -n "$USDT_RAW" ]; then
    USDT_VAL=$(printf "%.0f" "$USDT_RAW")
    MSG="${MSG}💲 **Tether (USDT):** $USDT_VAL تومان
"
else
    MSG="${MSG}💲 **Tether (USDT):** خطا
"
fi

if [ -n "$XRP" ]; then
    XRP_VAL=$(printf "%.4f" "$XRP")
    MSG="${MSG}🔗 **Ripple (XRP):** \$${XRP_VAL}
"
else
    MSG="${MSG}🔗 **Ripple (XRP):** خطا
"
fi

MSG="${MSG}
📈 منبع: CoinGecko & Wallex
🤖 ارسال خودکار هر ۳ ساعت"

echo "$MSG" | hermes send -q --to "telegram:-1004304844769"
```

> ⚠️ **Gotcha**: Never use `$(printf ...)` directly inside a `\"$VAR\"` string that's already inside another `$(...)` or a heredoc — the nested `$()` confuses bash and causes syntax errors. **Always compute formatted values into variables first** (e.g., `BTC_VAL=$(printf "%.2f" "$BTC")`) and then reference the variable.

**Key insights from session:**
- Use `no_agent: true` with a script file for cron jobs that just send data — no LLM needed
- Script must live in `~/.hermes/scripts/` and be executable (`chmod +x`)
- Nobitex returns price in Rial×10 → divide by 10 for Tomans
- `hermes send --to telegram:<chat_id>` targets specific channel/group (negative ID for channels)
- Cron delivery: `deliver: "telegram:<chat_id>"` works but script piping to `hermes send` is more reliable

### Multi-Post Photo-Caption Pattern (Session Pattern)
When sending multiple distinct posts that each require an attached banner image with caption (e.g., Daily Tech/AI news, C++ tips, GitHub trends):

**Problem**: Using standard `MEDIA:` output from cron stdout without `no_agent: true` causes the LLM agent to summarize stdout, separating images from text. Setting `no_agent: true` with `MEDIA:` in stdout sends all images together then all text together.

**Fix / Best Pattern**: Use a Python script with `no_agent: true` that writes individual files containing the `MEDIA:/path/to/image.png` prefix followed by the text caption, and then schedule separate cron jobs to print each file individually. This guarantees proper Photo Caption attachment on Telegram and splits them into distinct posts with custom timing.

#### 📁 1. The Script (Generates Assets & Messages):
```python
# Script: ~/.hermes/scripts/send_tech_posts_separately.py
import urllib.request
import json
import subprocess

# Generate banners, fetch news, etc.
# Write output files with MEDIA prefix embedded:
with open("/tmp/msg1.txt", "w") as f:
    f.write("MEDIA:/tmp/banner_news.png\n🤖 **[AI News]** ...")

with open("/tmp/msg2.txt", "w") as f:
    f.write("MEDIA:/tmp/banner_cpp.png\n⚡ **[C++ Tip]** ...")
```

#### ⏰ 2. The Cron Jobs (Scheduled Individually):
```bash
# ~/.hermes/cron/jobs.json entries
# Job 1: 30 3 * * * (7:00 AM Tehran) -> Run send_tech_posts_separately.py, Output /tmp/msg1.txt
# Job 2: 31 3 * * * (7:01 AM Tehran) -> Output /tmp/msg2.txt (no_agent: true, file read)
# Job 3: 32 3 * * * (7:02 AM Tehran) -> Output /tmp/msg3.txt (no_agent: true, file read)
```

**Critical Gotchas**:
- **Photo Caption**: Use `no_agent: true` with the `MEDIA:` prefix embedded as the very first line of your file content. This tells the Telegram adapter to send it as a photo-caption rather than a separate image file.
- **Tehran Time Sync**: UTC is 3:30 hours behind Tehran (UTC+3:30). To schedule at **7:00 AM Tehran**, set cron to **3:30 AM UTC** (`30 3 * * *`).
- **Separation**: To prevent Telegram from collapsing multiple posts into a single media group, separate their execution times by at least 1 minute (`03:30`, `03:31`, `03:32` UTC).
- **Tailwind CSS v4 syntax**: In newer Tailwind v4 environments, replace old directives (`@tailwind base;`) with `@import 'tailwindcss';` to avoid empty page render bugs during Vite build.

## Pitfalls & Gotchas

1. **No interactive auth**: `hermes send` uses pre-configured gateway credentials — run `hermes setup` or configure `~/.hermes/.env` first
2. **Chat ID format**: Telegram supergroups/channels use negative IDs (`-100...`), DMs use positive
3. **Rate limits**: Telegram ~30 msg/sec per bot; batch if sending multiple
4. **Markdown escaping**: Underscores, asterisks in data can break formatting — escape or use code blocks
5. **Cron environment**: PATH may be minimal; use full paths (`/usr/local/bin/hermes`) or source profile
6. **Approval prompts**: In some environments, `hermes send` may trigger approval — run interactively once to accept
7. **`hermes send` prints "sent" to stdout** — use `-q` (quiet) flag to suppress when running from cron scripts, otherwise the cron job output log includes unwanted line
8. **Iranian exchange DNS may break**: Nobitex DNS (`api.nobitex.ir`) is unreliable from outside Iran. Wallex (`api.wallex.ir`) works globally and returns Toman directly (no ÷10). Test API reachability before deploying as a cron script
9. **API response may contain null bytes**: Some external APIs (like Wallex, GoldAPI) occasionally return responses with embedded null bytes (`\x00`). This causes `jq` to fail with `parse error: Invalid numeric literal` because `jq` cannot handle null bytes in input. **Fix**: Sanitize API response before piping to `jq` using `tr -d '\000'` or `sed 's/\x00//g'` or Python's `.replace('\x00', '')` before parsing.

### Example fix for Wallex API null bytes:
```bash
WALLEX=$(curl -sf "https://api.wallex.ir/v1/markets" 2>/dev/null | tr -d '\000' || echo '{}')
USDT_RAW=$(echo "$WALLEX" | jq -r '.result.symbols.USDTTMN.stats.lastPrice // empty')
```

## Verification

```bash
# Test delivery
hermes send --to telegram "Test from cron-notifications skill"

# List configured targets
hermes send --list
```

## References

- `references/telegram-setup.md` — Telegram bot creation and chat ID discovery
- `references/message-formatting.md` — Markdown/emoji guide per platform
- `references/cron-environment.md` — PATH, env vars, logging in cron context
- `references/nobitex-api.md` — Nobitex API (deprecated, DNS unreliable outside Iran)
- `references/wallex-api.md` — Wallex API (preferred, returns Toman directly, works globally)
- `references/crypto-prices-debugging.md` — Debugging session: crypto prices script fixes (GoldAPI, Nginx 403, bot token mismatch)