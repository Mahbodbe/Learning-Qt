#!/bin/bash
# crypto-price-notify.sh
# Fetch crypto prices from CoinGecko and send to Telegram via Hermes
# Usage: ./crypto-price-notify.sh [chat_id]
# Chat ID optional - uses default from Hermes config if omitted

set -euo pipefail

# Configuration
CHAT_ID="${1:-}"  # Optional: telegram:-1004304844769
TARGET="${CHAT_ID:-telegram}"  # Default to home channel
COINS="bitcoin,tether,ripple"
CURRENCIES="usd,irr"
API_URL="https://api.coingecko.com/api/v3/simple/price"

# Colors for terminal output (not sent to Telegram)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log() { echo -e "${GREEN}[$(date '+%H:%M:%S')]${NC} $*"; }
warn() { echo -e "${YELLOW}[$(date '+%H:%M:%S')] WARNING:${NC} $*"; }
error() { echo -e "${RED}[$(date '+%H:%M:%S')] ERROR:${NC} $*" >&2; }

# Fetch prices
log "Fetching prices from CoinGecko..."
RESPONSE=$(curl -sf "${API_URL}?ids=${COINS}&vs_currencies=${CURRENCIES}" 2>/dev/null) || {
    error "Failed to fetch from CoinGecko"
    hermes send --to "${TARGET}" "🚨 **Crypto Price Alert Failed**\n\nFailed to fetch prices from CoinGecko API\nTime: $(TZ=Asia/Tehran date '+%Y/%m/%d %H:%M:%S')"
    exit 1
}

# Parse JSON (requires jq)
BTC_USD=$(echo "$RESPONSE" | jq -r '.bitcoin.usd // 0')
XRP_USD=$(echo "$RESPONSE" | jq -r '.ripple.usd // 0')
USDT_IRR=$(echo "$RESPONSE" | jq -r '.tether.irr // 0')

# Fallback for USDT/IRR if not available
if [[ "$USDT_IRR" == "0" || -z "$USDT_IRR" ]]; then
    USDT_USD=$(echo "$RESPONSE" | jq -r '.tether.usd // 1')
    USDT_IRR=$(awk "BEGIN {printf \"%.0f\", $USDT_USD * 42000}")
    warn "USDT/IRR not available, using fallback: ${USDT_IRR}"
fi

# Format numbers with commas
fmt() { printf "%'.2f" "$1" 2>/dev/null || printf "%.2f" "$1"; }
fmt_int() { printf "%'.0f" "$1" 2>/dev/null || printf "%.0f" "$1"; }

BTC_FMT=$(fmt "$BTC_USD")
XRP_FMT=$(fmt "$XRP_USD")
USDT_FMT=$(fmt_int "$USDT_IRR")

# Iran timestamp
TIMESTAMP=$(TZ=Asia/Tehran date '+%Y/%m/%d %H:%M:%S')

# Build message
MESSAGE=$(cat <<EOF
💰 **قیمت‌های لحظه‌ای ارزهای دیجیتال**

₿ **Bitcoin (BTC)**: \$${BTC_FMT}
💵 **Tether (USDT)**: ${USDT_FMT} تومان
💧 **Ripple (XRP)**: \$${XRP_FMT}

🕐 **زمان**: ${TIMESTAMP} (وقت ایران)
📊 **منبع**: CoinGecko API
EOF
)

# Send to Telegram
log "Sending to Telegram (target: ${TARGET})..."
hermes send --to "${TARGET}" "${MESSAGE}"

log "Done!"