# Nobitex API Reference (Iranian Crypto Exchange)

Used for USDT/IRR (Tether to Iranian Rial) pricing in cron notifications.

## Base URL
```
https://api.nobitex.ir
```

## Relevant Endpoints

### Orderbook (Best for spot price)
```
GET /v2/orderbook/USDTIRT
```

**Response:**
```json
{
  "status": "ok",
  "lastTradePrice": "4250000",
  "lastUpdate": 1699999999,
  "bids": [...],
  "asks": [...]
}
```

- `lastTradePrice`: Latest trade price in **Rials × 10** (e.g., 42500000 = 4,250,000 Rials = 425,000 Tomans)
- Convert to Tomans: `price / 10`

### Market Stats (Alternative)
```
GET /v2/market/stats?srcCurrency=usdt&dstCurrency=irt
```

**Response:**
```json
{
  "usdt-irt": {
    "latest": "42500000",
    "high": "43000000",
    "low": "42000000",
    "open": "42200000",
    "close": "42500000",
    "volume": "12345.67"
  }
}
```

- `latest`: Same format — Rials × 10

## Rate Limits
- No public docs on exact limits — be respectful
- In practice: ~30 req/min works fine for cron every 3h

## Notes
- No API key needed for public endpoints
- Iranian IPs may have better access; from outside Iran, may need proxy
- Prices in Rials × 10 — always divide by 10 for Tomans
- Best combined with CoinGecko for BTC/XRP/USD pairs

## Example Bash
```bash
RAW=$(curl -sf "https://api.nobitex.ir/v2/orderbook/USDTIRT" | jq -r '.lastTradePrice // empty')
if [ -n "$RAW" ]; then
    TOMAN=$(echo "scale=0; $RAW / 10" | bc)
    echo "$TOMAN تومان"
fi
```