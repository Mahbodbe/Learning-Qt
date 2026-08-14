#!/bin/bash
set -e

# Fetch data
# Use GoldAPI for XAU price
GOLD_DATA=$(curl -sf "https://api.gold-api.com/price/XAU" || echo '{"price": 2350}')
GOLD_OZ=$(echo "$GOLD_DATA" | jq -r '.price // 2350')

COINGECKO=$(curl -sf "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ripple&vs_currencies=usd" || echo '{}')
WALLEX=$(curl -sf "https://api.wallex.ir/v1/markets" || echo '{}')

# Iran time
NOW=$(TZ=Asia/Tehran date '+%Y-%m-%d %H:%M')

# Parse
BTC=$(echo "$COINGECKO" | jq -r '.bitcoin.usd // empty')
XRP=$(echo "$COINGECKO" | jq -r '.ripple.usd // empty')
USDT_RAW=$(echo "$WALLEX" | jq -r '.result.symbols.USDTTMN.stats.lastPrice // empty')

# Calculations
GOLD_GRAM_USD=$(echo "scale=2; $GOLD_OZ / 31.1035" | bc)
GOLD_GRAM_TMN=$(echo "scale=0; $GOLD_GRAM_USD * $USDT_RAW" | bc)

# Build message
MSG="📊 **گزارش بازار**
⏰ $NOW (وقت ایران)

₿ **Bitcoin:** \$$(printf "%'.2f" "$BTC")
💲 **Tether:** $(printf "%'.0f" "$USDT_RAW") تومان
🔗 **Ripple:** \$$(printf "%'.4f" "$XRP")
🥇 **Gold (per gram):**
   💵 $GOLD_GRAM_USD USD
   💲 $(printf "%'.0f" "$GOLD_GRAM_TMN") تومان

💡 **تحلیل کوتاه:**"

if (( $(echo "$USDT_RAW > 170000" | bc -l) )); then MSG="$MSG 💲 تتر در س ট্র در سقف."; else MSG="$MSG � تتر نرمال."; fi
if (( $(echo "$GOLD_OZ > 2350" | bc -l) )); then MSG="$MSG 🥇 انس جهانی صعودی."; else MSG="$MSG 🥇 انس جهانی اصلاحی."; fi
if (( $(echo "$GOLD_GRAM_TMN > 9000000" | bc -l) )); then MSG="$MSG 🥇 طلا هر گرم در سقف."; else MSG="$MSG 🥇 طلا هر گرم نرمال."; fi

MSG="$MSG

📈 منبع: GoldAPI, CoinGecko & Wallex
🤖 ارسال خودکار هر ۳ ساعت"

echo "$MSG" | hermes send -q --to "telegram:-1004304844769"