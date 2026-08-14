# Reproduction & Fix Recipe: 429 RESOURCE_EXHAUSTED Co-occurrence Bug

## Reproduction Test Template

Test the exact trigger with this template, varying only the two identity strings:

```
You are {a}, an intelligent AI assistant created by {b}. You are helpful, knowledgeable, and direct.
```

| Variant | Result | Latency |
|---|---|---|
| `a="Hermes Agent"`, `b="Nous Research"` | **FAIL 429, 4/4** | ~31.6-31.8s |
| neither string | OK, 2/2 | ~1.3-1.8s |
| only `"Hermes Agent"` | OK | ~1.5s |
| only `"Nous Research"` | OK | ~1.8s |

Deterministic across every run. Failures hang ~32s, successes return in under 2s. A real quota rejection returns immediately — the ~32s wait is the signature of this content-filter bug.

## Fix Recipe (3 files, not 1)

⚠️ **CRITICAL PITFALL:** Fixing only `~/.hermes/SOUL.md` is NOT enough. The identity string is hardcoded in THREE locations in hermes-agent source. All three must be patched:

### File 1: `~/.hermes/SOUL.md` (line 1)

Change:
```
You are Hermes Agent, an intelligent AI assistant created by Nous Research. You are helpful, knowledgeable, and direct...
```
to:
```
You are Hermes, an intelligent AI assistant. You are helpful, knowledgeable, and direct...
```

### File 2: `/usr/local/lib/hermes-agent/agent/prompt_builder.py`

Find line ~127 (DEFAULT_AGENT_IDENTITY constant):
```python
DEFAULT_AGENT_IDENTITY = (
    "You are Hermes Agent, an intelligent AI assistant created by Nous Research. "
    ...
)
```
Change to:
```python
DEFAULT_AGENT_IDENTITY = (
    "You are Hermes, an intelligent AI assistant. "
    ...
)
```

### File 3: `/usr/local/lib/hermes-agent/hermes_cli/default_soul.py`

Same fix pattern — find the `DEFAULT_IDENTITY` string and remove either `"Hermes Agent"` or `"Nous Research"`.

### Verification After Fix

Send a test prompt and check latency:
- **Before fix:** ~32s timeout + 429 RESOURCE_EXHAUSTED
- **After fix:** <2s successful completion

If 9router HTTP 503 persists after identity fix: these are **local backoff errors** (configurable 7s in 9router), NOT upstream errors. Disabling Antigravity won't fix them — adjust 9router backoff config in its dashboard.

## What I Ruled Out (so future investigators don't repeat)

- **Quota / TPM exhaustion** — wrong. Requests that fail succeed seconds later with one word changed.
- **`max_tokens`** — tested 512 through 65536. Upstream 429 at every size.
- **Streaming** — fails identically with `stream: true` and `false`.
- **Prompt length** — controlled for; length is not the variable. Scrubbing the two names from an otherwise identical prompt fixes it.
- **`projectId`** — see original issue; a real bug, but not this one.
- **Model choice** — same behavior across Antigravity models.

## Source

- GitHub Issue: https://github.com/decolua/9router/issues/2964
- Root cause found by: mohvahedi (2026-08-03)
- Comment ID: 5161082471 (most useful analysis)
