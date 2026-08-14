# Crypto Price Script Debugging

## CoinGecko Gold API Behavior

`api.coingecko.com/api/v3/simple/price?ids=gold&vs_currencies=usd` returns the price of gold **per gram in USD**, NOT per troy ounce.

- Raw response: `{"gold":{"usd":0.00002327}}` — this is per gram, very small number
- To get per-ounce: multiply by 31.1035
- To get a realistic price: use `api.gold-api.com/price/XAU` which returns the ounce price directly (e.g. ~2350)

**Lesson:** CoinGecko's `gold` ticker is unreliable for gold prices in a production dashboard. Use GoldAPI or similar.

## Wallex API — Tether to Toman

```json
{
  "result": {
    "symbols": {
      "USDTTMN": {
        "stats": {
          "lastPrice": "173530.0000000000000000",
          "bidPrice": "173539.0000000000000000",
          "askPrice": "173540.0000000000000000"
        }
      }
    }
  }
}
```

Access: `jq -r '.result.symbols.USDTTMN.stats.lastPrice // empty'`

Wallex returns Toman directly (no division by 10 needed).

## Script Structure (no_agent cron)

```bash
#!/bin/bash
set -e

# Data sources
COINGECKO=$(curl -sf "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin,ripple&vs_currencies=usd" || echo '{}')
WALLEX=$(curl -sf "https://api.wallex.ir/v1/markets" || echo '{}')
GOLD=$(curl -sf "https://api.gold-api.com/price/XAU" || echo '{"price": 2350}')

# Parsing
BTC=$(echo "$COINGECKO" | jq -r '.bitcoin.usd // empty')
USDT=$(echo "$WALLEX" | jq -r '.result.symbols.USDTTMN.stats.lastPrice // empty')
GOLD_OZ=$(echo "$GOLD" | jq -r '.price // 2350')

# Gold per gram
GOLD_GRAM_USD=$(echo "scale=2; $GOLD_OZ / 31.1035" | bc)
GOLD_GRAM_TMN=$(echo "scale=0; $GOLD_GRAM_USD * $USDT" | bc)

# Build + deliver
echo "$MSG" | hermes send -q --to "telegram:-1004304844769"
```
