# Wallex API Reference (Iranian Crypto Exchange)

Used for USDT/TMN (Tether to Iranian Toman) pricing in cron notifications. Replaced Nobitex (DNS unreliable from outside Iran).

## Base URL
```
https://api.wallex.ir
```

## Relevant Endpoints

### Markets (Best for spot price)
```
GET /v1/markets
```

**Response (USDTTMN excerpt):**
```json
{
  "result": {
    "symbols": {
      "USDTTMN": {
        "symbol": "USDTTMN",
        "baseAsset": "USDT",
        "quoteAsset": "TMN",
        "stats": {
          "bidPrice": "173539.0000000000000000",
          "askPrice": "173540.0000000000000000",
          "lastPrice": "173530.0000000000000000",
          "24h_ch": -1.57,
          "24h_highPrice": "176893.0000000000000000",
          "24h_lowPrice": "173000.0000000000000000"
        }
      }
    }
  }
}
```

- `lastPrice`: Latest trade price in **Toman** (direct — no division needed!)
- Also available: XRP/TMN, BTC/TMN, ETH/TMN

### Individual Symbol (Alternative)
```
GET /v1/market?symbol=USDTTMN
```

## Key Differences from Nobitex

| Aspect | Nobitex | Wallex |
|--------|---------|--------|
| Price unit | Rial × 10 (÷10 for Toman) | Toman directly |
| DNS | Unreliable outside Iran | Works globally |
| Endpoint | `/v2/orderbook/USDTIRT` | `/v1/markets` |
| Auth | None needed | None needed |

## Rate Limits
- No public rate limits documented
- 1 req per 3 hours is well under any limit

## Notes
- No API key needed for public endpoints
- Works reliably from outside Iran
- Returns many pairs; extract `USDTTMN` from `.result.symbols`
- Best combined with CoinGecko for BTC/XRP/USD pairs

## Example Bash
```bash
WALLEX=$(curl -sf "https://api.wallex.ir/v1/markets")
USDT_RAW=$(echo "$WALLEX" | jq -r '.result.symbols.USDTTMN.stats.lastPrice // empty')
if [ -n "$USDT_RAW" ]; then
    echo "$(printf "%.0f" "$USDT_RAW") تومان"
fi
```
