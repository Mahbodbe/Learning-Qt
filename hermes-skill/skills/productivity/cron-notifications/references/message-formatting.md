# Message Formatting Guide for Hermes send

## Platform Support Matrix

| Feature | Telegram | Discord | Slack | Signal |
|---------|----------|---------|-------|--------|
| **Bold** | `**text**` | `**text**` | `*text*` | ❌ |
| *Italic* | `__text__` | `*text*` | `_text_` | ❌ |
| `Code` | `` `text` `` | `` `text` `` | `` `text` `` | ❌ |
| ```Code Block``` | ``` ```code``` ``` | ``` ```code``` ``` | ``` ```code``` ``` | ❌ |
| > Quote | `> text` | `> text` | `> text` | ❌ |
| [Link](url) | `[text](url)` | `[text](url)` | `<url\|text>` | ✅ (raw) |
| Emoji | ✅ Native | ✅ Native | ✅ Native | ✅ Native |
| Mentions | `@username` | `<@user_id>` | `<@user_id>` | ❌ |
| Channel ref | `#channel` | `#channel` | `#channel` | ❌ |

## Best Practices

### 1. Escape Special Characters

```bash
# Bad - underscores break markdown
hermes send --to telegram "Price: $100_000"

# Good - escape or use code block
hermes send --to telegram "Price: \$100\_000"
hermes send --to telegram "\`\`\`Price: $100_000\`\`\`"
```

### 2. Persian/RTL Text

```bash
# Use explicit RTL markers or rely on platform
hermes send --to telegram "💰 **قیمت**: ۱۲۳,۴۵۶ تومان"
# Telegram handles RTL automatically
```

### 3. Number Formatting

```bash
# Add commas for readability
python3 -c "print(f'{1234567:,}')"  # 1,234,567

# In bash
printf "%'d\n" 1234567  # 1,234,567 (locale-dependent)
```

### 4. Timestamps

```bash
# Iran time
TZ=Asia/Tehran date +"%Y/%m/%d %H:%M:%S"
# 2026/07/04 13:06:43

# UTC ISO
date -u +"%Y-%m-%dT%H:%M:%SZ"
# 2026-07-04T09:36:43Z
```

### 5. Structured Messages

```bash
# Header + body + footer pattern
cat << 'EOF' | hermes send --to telegram --file -
🚨 **ALERT: Service Down**

Service: nginx
Host: web-01
Since: 2026-07-04 13:05:00 UTC

Action: Auto-restart attempted
Status: ❌ Failed (3 retries)

🔗 Dashboard: https://grafana.example.com/d/nginx
EOF
```

### 6. Media Attachments

```bash
# Image
hermes send --to telegram "MEDIA:/tmp/chart.png"

# Document
hermes send --to telegram "MEDIA:/tmp/report.pdf"

# Multiple (separate messages)
for f in /tmp/*.png; do hermes send --to telegram "MEDIA:$f"; done
```

## Crypto Price Message Template

```bash
# Persian crypto prices
MESSAGE=$(cat << 'EOF'
💰 **قیمت‌های لحظه‌ای ارزهای دیجیتال**

₿ **Bitcoin (BTC)**: $62,408.00
💵 **Tether (USDT)**: 41,963 تومان
💧 **Ripple (XRP)**: $1.14

🕐 **زمان**: 2026/07/04 13:06:43 (وقت ایران)
📊 **منبع**: CoinGecko API
EOF
)
hermes send --to telegram "$MESSAGE"
```

## Common Emoji Reference

| Category | Emojis |
|----------|--------|
| Money | 💰 💵 💸 💲 🪙 ₿ |
| Status | ✅ ❌ ⚠️ 🚨 🔄 ⏳ |
| Direction | 📈 📉 ➡️ ⬅️ ⬆️ ⬇️ |
| Time | 🕐 📅 ⏰ 🗓️ |
| Tech | 🤖 🖥️ 📊 🔧 ⚙️ |
| Crypto | ₿ 💎 🚀 🌙 🐋 |

## Length Limits

| Platform | Limit |
|----------|-------|
| Telegram | 4096 chars |
| Discord | 2000 chars |
| Slack | 40000 chars (blocks) |
| Signal | ~4000 chars |

Split long messages if needed.