# Session 2026-07-11: Full Server State to GitHub + Instrumentation Project Updates

## Migration: push ALL to GitHub

User directive: "همه چیز همه همه چیز هیچی نباشه که فرستاده نشه روی گیت‌هاب" — zero filtering.

### Excluded only by GitHub's 100 MB limit:
- `.hermes/node/bin/node` (119 MB)  
- `.hermes/bin/uv` (61 MB)  

These were added to `.gitignore`. Re-download with `hermes setup` on the new server.

### What the git repo now contains:
- Entire `/root/resin-web/` (Django project + website DB)
- Entire `/root/.hermes/` (config, state.db, memories, skills, kanban, cron)
- `docs/` — instrumentation project files (.docx, .pdf)
- Full backup at `/root/resin-web-full-backup-20260711-175610.tar.gz` (138MB)

## Slack space cleaned (~1.8 GB freed)

| What | Size | Command |
|------|------|---------|
| `/root/.cache/*` | ~710 MB | `rm -rf /root/.cache/*` |
| `journalctl --vacuum-size=100M` | ~1.1 GB | `journalctl --vacuum-size=100M` |

Result: 96% → 87% full.

## Rejected cleanup proposals (list before acting rule)

User enforced: **list all items first with explanation, get confirm, then act in one batch**.

Items listed but user chose to skip (only old backups deleted):
- `/var/cache/apt` (599 MB) — apt package cache
- `/var/log/` (270 MB) — syslog/kern.log/btmp
- docker container prune (16 KB)

## Celery 5.x fix

`run.sh` used `--daemon` flag (removed in Celery 5.x). Fixed with `nohup` backgrounding:

```bash
nohup celery -A website beat -l WARNING > /tmp/celery-beat.log 2>&1 &
nohup celery -A website worker -l INFO --concurrency 1 > /tmp/celery-worker.log 2>&1 &
```

Also set `concurrency 1` for 1 GB RAM VPS.

## PromotionQueueItem fix

`views.py:send_order_to_bot()` was creating `PromotionQueueItem` for every order — these polluted the channel queue. **Fix:** remove line `PromotionQueueItem.objects.create(product=product)`, keep only admin notification.

Admin list: user rejected `get_queryset` default-filter. Keep `list_filter = ("status", "created_at")` — user manually selects PENDING from dropdown.

## DOCX generation with B Nazanin

When `python-docx` unavailable (pip timeout), fallback: `unzip` → `sed` on `word/styles.xml` → `zip` re-pack.
See `references/docx-fallback-patching.md`.
