---
name: vps-memory-ops
description: Memory/swap management and safe service control on the ~961MB-RAM VPS that hosts Mahgol Resin + Hermes. Reusable recipe for swap resize, OOM avoidance, and which services are off-limits.
---

# Low-RAM VPS Memory Operations

## Context
Deployment VPS: ~961 MB RAM, originally a 2 GB `/swapfile`. Co-resident: gunicorn (Mahgol Resin), next-server (9router), hermes-gateway, redis, nginx, x-ui/xray, dockerd. OOM killer periodically kills gunicorn under load.

## Resize swap (reusable recipe)
Swap is a file at `/swapfile`. To grow 2G → 4G (fallocate is faster/cleaner than dd):
```
sudo swapoff /swapfile
sudo fallocate -l 4G /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile
free -h
cat /proc/swaps
```
- `mkswap` will warn "wiping old swap signature" — expected.
- To clear swap cache WITHOUT resizing: `sudo swapoff -a && sudo swapon -a`.

## OOM avoidance (durable)
- Keep gunicorn at `--workers 1` (memory-limited; more workers → OOM).
- Grow swap BEFORE raising workers or adding services.
- Watch: `free -m`, `dmesg | grep -i oom`.

## SERVICES YOU MUST NOT KILL (hard constraints)
- **docker** — user: "داکر رو بکشی خودت قطع میشی" (killing docker disconnects your own session). NEVER `systemctl stop docker` / kill dockerd without explicit user OK.
- **9router** (next-server, ~14% RAM, top consumer) — user EXEMPTED it: "هرچی داره زیاد مصرف میکنه رو جز 9router آف کن" means keep 9router, kill the rest. Off-limits.
- **hermes-gateway** — that's this agent. Don't kill yourself.

## Services usually safe to trim — BUT ASK FIRST
snapd, cups, ModemManager, udisks2 are not needed on a headless VPS, yet the user said "اون یکیا هم لازم بوده حتما" (those were needed) — so DO NOT assume; confirm before disabling any.

## 9router Service
9router runs on port 20128. It uses Next.js under the hood and requires Node.js v22 (hermes node binary) for compatibility with the bundled better-sqlite3 native module.

**Startup command (bypassing npx which downloads on each run):**
```bash
pkill -9 -f 9router 2>/dev/null
pkill -9 -f "npm.*9router" 2>/dev/null
sleep 3
cd /root/.9router && nohup /usr/bin/node9router /usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128 > /dev/null 2>&1 & disown
```

**Verify:**
```bash
ss -tlnp | grep 20128
curl -s -o /dev/null -w "%{http_code}" http://localhost:20128/
# Should return 307 (redirect to login)
```

**Systemd service file:**
```ini
[Unit]
Description=9router Next.js Server
After=network.target

[Service]
Type=simple
ExecStart=/usr/bin/node9router /usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128
Restart=always
RestartSec=5
User=root

[Install]
WantedBy=multi-user.target
```

**Key:** Uses `/usr/bin/node9router` (symlink to hermes Node v22) instead of system node (v18) to avoid native module version mismatches (better-sqlite3).

## 9router File Dependencies
9router requires these files in `/root/.9router/`:
- `db/data.sqlite` — SQLite database (auth, settings, usage history)
- `auth/` — auth configuration
- `jwt-secret` — JWT signing key
- `machine-id` — unique machine identifier
- `runtime/` — Next.js runtime dependencies (sql.js, better-sqlite3, etc.)

After migrating servers, ensure all these are copied:
```bash
rsync -avz /root/.9router/auth/ root@new-server:/root/.9router/auth/
scp /root/.9router/jwt-secret /root/.9router/machine-id root@new-server:/root/.9router/
rsync -avz /root/.9router/runtime/ root@new-server:/root/.9router/runtime/
```

## 9router Common Issues

### 500 Internal Server Error on all endpoints
**Causes:**
1. **Node version mismatch** — Next.js compiled with Node 22 but running on Node 18. Fix: Use `node9router` (hermes Node v22) as shown above.
2. **Missing database files** — `data.sqlite` missing or corrupted. Copy from old server.
3. **Missing runtime deps** — `sql.js` or `better-sqlite3` missing in `/root/.9router/runtime/node_modules/`. Copy `/root/.9router/runtime/` from old server.

### 9router won't start / command hangs
**Cause:** `npx` tries to download 9router on each run (slow/fails). The `9router` CLI is at `/usr/local/lib/node_modules/9router/cli.js`.
**Fix:** Run directly with correct Node:
```bash
/usr/bin/node9router /usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128
```

### Systemd service keeps restarting (npx version)
The old systemd service used `ExecStart=/usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128` but the CLI wrapper tries to use `npx` internally. Systemd restarts it infinitely.
**Fix:** Update service to use direct Node binary:
```ini
ExecStart=/usr/bin/node9router /usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128
```

## 9router File Dependencies Transfer
After migrating servers, ensure all these are copied:
```bash
rsync -avz /root/.9router/auth/ root@new-server:/root/.9router/auth/
scp /root/.9router/jwt-secret /root/.9router/machine-id root@new-server:/root/.9router/
rsync -avz /root/.9router/runtime/ root@new-server:/root/.9router/runtime/
```

## 9router Common Issues

### 500 Internal Server Error on all endpoints
**Causes:**
1. **Node version mismatch** — Next.js compiled with Node 22 but running on Node 18. Fix: Use `node9router` (hermes Node v22) as shown above.
2. **Missing database files** — `data.sqlite` missing or corrupted. Copy from old server.
3. **Missing runtime deps** — `sql.js` or `better-sqlite3` missing in `/root/.9router/runtime/node_modules/`. Copy `/root/.9router/runtime/` from old server.

### 9router won't start / command hangs
**Cause:** `npx` tries to download 9router on each run (slow/fails). The `9router` CLI is at `/usr/local/lib/node_modules/9router/cli.js`.
**Fix:** Run directly with correct Node:
```bash
/usr/bin/node9router /usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128
```

### Systemd service keeps restarting (npx version)
The old systemd service used `ExecStart=/usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128` but the CLI wrapper tries to use `npx` internally. Systemd restarts it infinitely.
**Fix:** Update service to use direct Node binary:
```ini
ExecStart=/usr/bin/node9router /usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128
```

## 9router File Dependencies Transfer
After migrating servers, ensure all these are copied:
```bash
rsync -avz /root/.9router/auth/ root@new-server:/root/.9router/auth/
scp /root/.9router/jwt-secret /root/.9router/machine-id root@new-server:/root/.9router/
rsync -avz /root/.9router/runtime/ root@new-server:/root/.9router/runtime/
```

## 9router Common Issues

### 500 Internal Server Error on all endpoints
**Causes:**
1. **Node version mismatch** — Next.js compiled with Node 22 but running on Node 18. Fix: Use `node9router` (hermes Node v22) as shown above.
2. **Missing database files** — `data.sqlite` missing or corrupted. Copy from old server.
3. **Missing runtime deps** — `sql.js` or `better-sqlite3` missing in `/root/.9router/runtime/node_modules/`. Copy `/root/.9router/runtime/` from old server.

### 9router won't start / command hangs
**Cause:** `npx` tries to download 9router on each run (slow/fails). The `9router` CLI is at `/usr/local/lib/node_modules/9router/cli.js`.
**Fix:** Run directly with correct Node:
```bash
/usr/bin/node9router /usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128
```

### Systemd service keeps restarting (npx version)
The old systemd service used `ExecStart=/usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128` but the CLI wrapper tries to use `npx` internally. Systemd restarts it infinitely.
**Fix:** Update service to use direct Node binary:
```ini
ExecStart=/usr/bin/node9router /usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128
```

## 9router File Dependencies Transfer
After migrating servers, ensure all these are copied:
```bash
rsync -avz /root/.9router/auth/ root@new-server:/root/.9router/auth/
scp /root/.9router/jwt-secret /root/.9router/machine-id root@new-server:/root/.9router/
rsync -avz /root/.9router/runtime/ root@new-server:/root/.9router/runtime/
```

## 9router Common Issues

### 500 Internal Server Error on all endpoints
**Causes:**
1. **Node version mismatch** — Next.js compiled with Node 22 but running on Node 18. Fix: Use `node9router` (hermes Node v22) as shown above.
2. **Missing database files** — `data.sqlite` missing or corrupted. Copy from old server.
3. **Missing runtime deps** — `sql.js` or `better-sqlite3` missing in `/root/.9router/runtime/node_modules/`. Copy `/root/.9router/runtime/` from old server.

### 9router won't start / command hangs
**Cause:** `npx` tries to download 9router on each run (slow/fails). The `9router` CLI is at `/usr/local/lib/node_modules/9router/cli.js`.
**Fix:** Run directly with correct Node:
```bash
/usr/bin/node9router /usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128
```

### Systemd service keeps restarting (npx version)
The old systemd service used `ExecStart=/usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128` but the CLI wrapper tries to use `npx` internally. Systemd restarts it infinitely.
**Fix:** Update service to use direct Node binary:
```ini
ExecStart=/usr/bin/node9router /usr/local/lib/node_modules/9router/cli.js --tray --skip-update -p 20128
```
After a server reboot, the full Mahgol Resin stack must be restarted. `run.sh` exists but Celery 5.x+ no longer supports `--daemon`. Fixed startup sequence:

```bash
cd /root/resin-web && bash run.sh
# Then separately start Celery (run.sh fails for Celery in v5+):
cd /root/resin-web/website
source ../venv/bin/activate
nohup celery -A website beat -l WARNING > /tmp/celery-beat.log 2>&1 &
nohup celery -A website worker -l INFO --concurrency 1 > /tmp/celery-worker.log 2>&1 &
```

After starting, verify:
- `curl -s -o /dev/null -w "%{http_code}" http://localhost:8000/admin/` → should be 302 (redirect to login)
- `ps aux | grep gunicorn | grep -v grep` → should show master + worker
- `ps aux | grep celery | grep -v grep` → should show beat + worker
- `ps aux | grep bot.py | grep -v grep` → should show polling bot

If gunicorn returns HTTP 000: check `/tmp/gunicorn.log` for error details. Common issues: missing bind port, venv not activated, wrong wsgi module path.

## Celery Compatibility Note
Celery v5.4+ dropped the `--daemon` flag entirely. `celery multi start` also deprecated. Use shell background (`nohup ... &`) or systemd instead. The user's `run.sh` uses `--daemon` → fails silently for celery; gunicorn (v21) still supports `--daemon`.

## Pitfalls
- Don't `fallocate` a swap file while still `swapon`'d without `swapoff` first — mkswap warns and risks corruption.
- Don't kill docker to free RAM — you sever your own session.
- Don't assume a high-RAM process is safe to kill (9router is the biggest but exempt).
- Disk space (>90% usage on 19GB root) causes silent failures: gunicorn can't write logs, bot crashes on file write, git push rejects. Always check `df -h /` when user reports weird failures.
- Disk hogs to check when <10% free: `/root/.cache/` (~710MB), `/var/log/` (~345MB after vacuum), `/var/cache/apt` (~599MB), old `.tar.gz` backups. See `mahboob-persona` skill for the cleanup protocol (list → confirm → act).
- On this VPS, fixed-cost space is ~55% of 19GB already consumed: swapfile (4GB), snap (4GB), docker images (~800MB), .hermes (442MB), /usr (6.8GB). This leaves only ~8.5GB for project data and logs — fills fast.
- **Celery `--daemon` incompatibility**: if a `run.sh` or startup script uses `--daemon` for celery, it will FAIL silently on Celery v5.4+. Replace with `nohup ... &` pattern or systemd unit.
- **Gunicorn config file**: `/root/resin-web/gunicorn_config.py` does NOT exist. The repo uses command-line flags in `run.sh` directly. When migrating servers, the gunicorn startup args must be passed inline, not via a config file.
