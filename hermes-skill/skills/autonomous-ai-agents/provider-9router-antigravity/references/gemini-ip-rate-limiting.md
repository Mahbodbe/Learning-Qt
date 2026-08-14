# Gemini IP-Based Rate Limiting (9router Context)

**Discovered:** August 11, 2026
**User:** Mahbod (@Mahbodbe)

## Issue
Even with multiple Gemini API keys (e.g., 9 separate accounts) configured in 9router, the provider returns rapid 429/503 errors ("Resource Exhausted" or "Service Unavailable").

## Diagnosis
Google Gemini (API/Antigravity) implements rate limiting at multiple tiers. One primary tier is the **Source IP Address**. 
- 9router rotates between keys, but if the 9router instance is running on a single VPS/Server, all requests originate from that server's IP.
- Google detects the high volume from a single IP and applies a global throttle to that IP, regardless of the variety of API keys used.
- This effectively nullifies the benefit of key-rotation unless the requests are also distributed across multiple IPs.

## Workarounds
1. **Rotating Proxies:** Configure 9router or the upstream provider to route through a pool of rotating residential or data-center proxies.
2. **Cloudflare Gateway:** Route Gemini requests through a Cloudflare Worker or reverse proxy. While Google might still limit the CF IP range, the distribution is wider than a single VPS IP.
3. **vLLM / Aggregation Services:** Use services that handle IP rotation natively at the infrastructure layer.
4. **Local Backoff (9router):** 9router has a default 7s backoff on 503s. If the limit is hit, 9router might block locally even if Google would have accepted the next request. Check `9router` dashboard for "local backoff" metrics.

## Free Non-Google Alternatives (No-Card Requirements)
When Gemini rate limits are unresolvable and credit-card-free alternatives are needed:
1. **OpenRouter Free Tier:**
   - Offers free models (e.g., `meta-llama/llama-3.1-8b-instruct:free`).
   - Does not require credit card verification for free tier usage. Highly recommended for low-friction migration.
2. **HuggingFace Inference API:**
   - Free tier accessible via standard HuggingFace account tokens (no card required).
   - Generous rate limits (up to 30k tokens/min) for smaller open models.
3. **Self-Hosted Local Models (Hardware Constraints):**
   - Hosting local models via `vllm` or `llama.cpp` requires at least 8GB-16GB RAM and preferably a GPU.
   - **Do not attempt** to host local LLMs on low-resource VPS environments (e.g., 1GB RAM, Swap enabled, no GPU), as it will fail due to out-of-memory (OOM) errors.

