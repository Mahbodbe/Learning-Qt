---
name: provider-9router-antigravity
description: "Configure Hermes Agent for 9router + Antigravity (Google backend) — fix SOUL.md identity co-occurrence triggering 429 RESOURCE_EXHAUSTED @ ~32s, and handle 9router local 503 backoff."
category: autonomous-ai-agents
tags: [9router, antigravity, provider-integration, hermes-identity, hermes-config]
version: 1.1.0
author: Hermes Agent
license: MIT
platforms: [linux, macos, windows]
---

# Provider Integration: 9router / Antigravity with Hermes Agent

## Problem: SOUL.md Identity Co-occurrence Triggers Upstream 429 RESOURCE_EXHAUSTED

**Discovered by:** mohvahedi (github.com/decolua/9router/issues/2964)

### Root Cause
When Hermes Agent's default identity string contains **BOTH** the "Hermes Agent" string **AND** the "Nous Research" string **simultaneously**, the Antigravity/Google backend returns `429 RESOURCE_EXHAUSTED` after exactly **~32 seconds** latency.

Each string **alone** is harmless; only their **co-occurrence** triggers the upstream rate limit.

### Latency Signature
| Identity Strings | Result | Latency |
|-----------------|--------|---------|
| Both present | **FAIL 429** (deterministic) | ~32 seconds |
| Either one only | OK | ~1.5–1.8s |
| Neither | OK | ~1.5–1.8s |

The ~32s timeout is characteristic: upstream Google API hangs before rejecting with `429 RESOURCE_EXHAUSTED` (misleading code — actually a content-based filter, not quota).

### Affected Files (3 locations)
1. `~/.hermes/SOUL.md` (line 1 — identity string)
2. `agent/prompt_builder.py` (prompt template)
3. `hermes_cli/default_soul.py` (default identity)

### Fix
Edit `~/.hermes/SOUL.md` line 1 to **remove one of the two trigger strings** — e.g. change:
```
You are Hermes Agent, an intelligent AI assistant created by Nous Research...
```
to:
```
You are Hermes, an intelligent AI assistant...
```
(Remove either the AGENT-name string or the NOUS-RESEARCH string.)

Also patch:
- `agent/prompt_builder.py`
- `hermes_cli/default_soul.py`

If 9router HTTP 503 persists after identity fix: these are **local backoff errors** (configurable 7s in 9router), NOT upstream errors. Disabling Antigravity won't fix them — adjust 9router backoff config.

## 9router Local 503 Errors
9router's own 503s are **local backoff** (configurable 7s in 9router), NOT upstream Google errors. Dashboard shows "100% Full Quota" but live requests hit 1-minute TPM limits. Disabling Antigravity won't fix these — adjust 9router backoff config in its dashboard.

## Gemini IP-Based Rate Limiting
Gemini (API/Antigravity) rate limits are often applied to the **Source IP**. Rotating API keys (e.g., 9 accounts) on a single VPS will still trigger 429/503 errors because the IP is the bottleneck. 
- **Diagnosis:** See `references/gemini-ip-rate-limiting.md` for details.
- **Solution:** Use rotating proxies or a distributed gateway (Cloudflare) to spread the request load across multiple IPs.

## 🛠️ Update (Bespoke Soul Rewriting)
If the user prefers a bespoke, highly compact, and direct assistant persona (e.g. sharp, direct, no-fluff tone like Captain/partner), completely rewrite `~/.hermes/SOUL.md` with different word ordering and layout. E.g.:
```
I'm Hermes, your AI partner — sharp, practical, and direct. I help with coding, analysis, creative work, research, and running tools. I speak plainly, own my limits, and focus on what actually helps you get things done. No fluff, no hedging. Just clean, efficient collaboration.
```
This is a robust and permanent fix since it completely breaks any potential trigger patterns.

## Verified Fix Applied (2026-08-06)
The full fix was applied to a live Hermes install:
1. `~/.hermes/SOUL.md` → rewritten to bespoke persona (no trigger strings)
2. `/usr/local/lib/hermes-agent/agent/prompt_builder.py` → `DEFAULT_AGENT_IDENTITY` changed to `"You are Hermes, an intelligent AI assistant."`
3. `hermes_cli/default_soul.py` → same change
- Note: `plugins/platforms/telegram/adapter.py` also had `"Hermes Agent" + "by Nous Research"` but patching was not completed in-session — check and patch if 429s persist.

## References
- GitHub Issue: https://github.com/decolua/9router/issues/2964
- Root cause analysis by: mohvahedi (2026-08-03)
- Verified latency: deterministic ~32s failure vs <2s success
