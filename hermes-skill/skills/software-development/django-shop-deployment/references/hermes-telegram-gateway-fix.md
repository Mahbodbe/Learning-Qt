# Hermes Telegram Gateway Fix (Installed Instance)

## Symptom
User: "why hermes gateway tg bot doesnt answer me?"

`hermes gateway status` shows service `active (running)`, but bot never replies.
Gateway log (`~/.hermes/logs/gateway.log`) repeats:

```
ERROR hermes_plugins.telegram_platform.adapter: [Telegram] Failed to connect to Telegram: name 'InlineQueryHandler' is not defined
Traceback (most recent call last):
  File ".../plugins/platforms/telegram/adapter.py", line 2922, in connect
    self._app.add_handler(InlineQueryHandler(self._handle_inline_query))
NameError: name 'InlineQueryHandler' is not defined
```

## Root Cause
In `/usr/local/lib/hermes-agent/plugins/platforms/telegram/adapter.py`,
the top-level `try: from telegram...` import block omitted
`InlineQueryHandler`, `InlineQueryResultArticle`, `InputTextMessageContent`.

They WERE imported inside `ensure_telegram_dependencies()` (a lazy re-import
helper), but the normal connect path uses `InlineQueryHandler` at module
runtime without calling that helper first → `NameError` → Telegram never
connects → bot silent (no crash surface to the user, just no replies).

## Fix
Add the three names to BOTH the top-level import and the ImportError stub block.

Top-level `try:`:
```python
    from telegram import Update, Bot, Message, InlineKeyboardButton, InlineKeyboardMarkup
    from telegram import InlineQueryResultArticle, InputTextMessageContent
    try:
        from telegram import LinkPreviewOptions
    except ImportError:
        LinkPreviewOptions = None
    from telegram.ext import (
        Application,
        CommandHandler,
        CallbackQueryHandler,
        MessageHandler as TelegramMessageHandler,
        ContextTypes,
        filters,
        InlineQueryHandler,
    )
    from telegram.constants import ParseMode, ChatType
    from telegram.request import HTTPXRequest
    TELEGRAM_AVAILABLE = True
except ImportError:
    TELEGRAM_AVAILABLE = False
    # ... existing Any-stubs ...
    InlineQueryResultArticle = None
    InputTextMessageContent = None
    # ...
    InlineQueryHandler = Any
```

## Verification performed (this session)
No gateway restart was allowed by the user, so runtime connect was NOT proven.
Static checks passed:
```bash
/usr/local/lib/hermes-agent/venv/bin/python -m py_compile adapter.py
# → COMPILE OK

/usr/local/lib/hermes-agent/venv/bin/python -c "from telegram.ext import InlineQueryHandler"
# → OK (symbol resolves against installed telegram SDK)

/usr/local/lib/hermes-agent/venv/bin/python -c "from telegram import InlineQueryResultArticle, InputTextMessageContent"
# → OK
```

## To make the bot live (user action)
```bash
hermes gateway restart
hermes gateway status     # expect no InlineQueryHandler traceback, "Telegram polling resumed"
```
Then send a Telegram message to confirm.

## Caveats
- The edit is in the **installed package tree** `/usr/local/lib/hermes-agent/`.
  `hermes update` will overwrite it. Report upstream (Nous Research).
- No pytest applies here — this is the installed lib, not the source repo
  (`scripts/run_tests.sh` targets the git checkout).
- The `NameError` was the SOLE failure cause; once the symbol resolves,
  adapter.py:2922 cannot re-raise it. Only unverified link is live network connect.

## Related: Nginx "off" but it was the upstream
Same session, user reported nginx off. Diagnosis:
- `systemctl status nginx` → `active (running)`.
- `ss -tlnp | grep -E ':80|:8000'` → `:80` listening (nginx) but `:8000` NOT.
- → Gunicorn/Django upstream was dead, that's why the site was unreachable,
  not nginx. Fix: start the stack (see SKILL.md "Start the Stack").
- After restart: `curl http://127.0.0.1:8000/` → 200, `curl http://domain/` → 200.
