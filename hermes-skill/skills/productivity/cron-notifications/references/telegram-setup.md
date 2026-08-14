# Telegram Bot Setup for Hermes

## Create Bot & Get Token

1. Open Telegram → search `@BotFather`
2. Send `/newbot` → follow prompts
3. Copy the **API token** (format: `123456789:ABC-DEF1234ghIkl-zyx57W2v1u123ew11`)

## Get Chat ID

### For Private Chat (DM)
1. Message your bot anything
2. Visit: `https://api.telegram.org/bot<TOKEN>/getUpdates`
3. Find `"chat":{"id":123456789}` → that's your chat ID

### For Group/Channel
1. Add bot to group/channel as admin
2. Send a message in the group
3. Visit same `getUpdates` URL
4. Find `"chat":{"id":-1001234567890}` → negative ID = supergroup/channel

## Configure Hermes

### Option A: Interactive setup
```bash
hermes setup
# Choose "Telegram" → enter token → enter chat ID
```

### Option B: Manual .env
```bash
# ~/.hermes/.env
TELEGRAM_BOT_TOKEN=123456789:ABC-DEF1234ghIkl-zyx57W2v1u123ew11
TELEGRAM_CHAT_ID=-1001234567890  # or positive for DM
```

### Option C: config.yaml (advanced)
```yaml
# ~/.hermes/config.yaml
gateways:
  telegram:
    bot_token: "123456789:ABC-DEF1234ghIkl-zyx57W2v1u123ew11"
    default_chat_id: "-1001234567890"
```

## Verify

```bash
hermes send --list telegram
# Should show: telegram:Your Name  [123456789]

hermes send --to telegram "Test message"
# "Sent to telegram home channel (chat_id: 123456789)"
```

## Common Issues

| Problem | Fix |
|---------|-----|
| "Unauthorized" | Bot token wrong or revoked |
| "Chat not found" | Chat ID wrong; bot not in group |
| "Can't send to channel" | Bot must be admin in channel |
| Approval prompt | Run once interactively to accept |