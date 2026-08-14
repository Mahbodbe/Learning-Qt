# Cron Environment Gotchas & Best Practices

## PATH Issues

Cron runs with minimal PATH (often just `/usr/bin:/bin`). Hermes and tools may not be found.

### Fix: Use Full Paths

```bash
# In crontab or script
/usr/local/bin/hermes send --to telegram "test"
# or
export PATH="/usr/local/bin:/usr/bin:/bin:$PATH"
```

### Fix: Source Profile

```bash
# At top of cron script
source ~/.profile
# or for zsh
source ~/.zshrc
```

## Environment Variables

Cron doesn't load `.bashrc`, `.zshrc`, or `.profile` by default.

### Option 1: BASH_ENV

```bash
# In crontab
BASH_ENV=/home/user/.bash_env
* * * * * /path/script.sh
```

```bash
# ~/.bash_env
export PATH="/usr/local/bin:$PATH"
export TELEGRAM_BOT_TOKEN=...
export TELEGRAM_CHAT_ID=...
```

### Option 2: Env File

```bash
# In script
set -a
source ~/.hermes/.env
set +a
```

### Option 3: hermes cron --env-file

```bash
hermes cron create \
  --name "daily-report" \
  --schedule "0 0 * * *" \
  --command "./report.sh" \
  --env-file ~/.secrets/report.env
```

## Working Directory

Cron runs from home directory (`~`) or `/` depending on system.

```bash
# Explicit cd in script
cd /path/to/project
# or
WORKDIR=$(dirname "$0")
cd "$WORKDIR"
```

## Logging & Output

By default, cron emails stdout/stderr to local mail (often lost).

### Redirect to File

```bash
# In crontab
* * * * * /path/script.sh >> /var/log/script.log 2>&1
```

### Use hermes cron logs

```bash
hermes cron logs job-name
# Shows stdout/stderr from last run
```

### Systemd-journald (modern systems)

```bash
journalctl -u cron -f
# or
journalctl -u hermes-cron -f
```

## Timezone

Cron uses system timezone (usually UTC). For Iran time scheduling:

### Option 1: CRON_TZ (GNU cron)

```bash
CRON_TZ=Asia/Tehran
0 0 * * * /script.sh  # Runs at midnight Iran time
```

### Option 2: TZ in Command

```bash
0 0 * * * TZ=Asia/Tehran /script.sh
```

### Option 3: Handle in Script

```bash
# Script checks Iran time
if [[ $(TZ=Asia/Tehran date +%H) -eq 0 ]]; then
    # Run at midnight Iran time
fi
```

## Concurrent Runs Protection

Prevent overlapping executions:

### flock (recommended)

```bash
#!/bin/bash
exec 9>/var/lock/my-script.lock
flock -n 9 || { echo "Already running"; exit 1; }

# ... actual work ...
```

### PID file

```bash
PIDFILE=/tmp/my-script.pid
if [[ -f $PIDFILE ]] && kill -0 $(cat $PIDFILE) 2>/dev/null; then
    exit 1
fi
echo $$ > $PIDFILE
trap "rm -f $PIDFILE" EXIT
```

## Debugging Cron Failures

### 1. Test Manually First

```bash
/path/script.sh  # Should work without cron
```

### 2. Check Cron Logs

```bash
grep CRON /var/log/syslog
# or
journalctl -u cron
```

### 3. Capture Output

```bash
* * * * * /script.sh 2>&1 | tee -a /tmp/cron-debug.log
```

### 4. Common Failure Modes

| Symptom | Cause | Fix |
|---------|-------|-----|
| "Command not found" | PATH missing | Use full paths |
| "Permission denied" | Script not executable | `chmod +x script.sh` |
| Works manually, fails in cron | Env vars missing | Source `.env` or use `BASH_ENV` |
| No output in Telegram | Chat ID wrong | `hermes send --list` to verify |
| Script hangs | stdin/stdout issues | Redirect: `</dev/null >/dev/null 2>&1` |

## hermes cron Specifics

```bash
# Create
hermes cron create --name job --schedule "0 * * * *" --command "cmd"

# List
hermes cron list

# View logs
hermes cron logs job --lines 100

# Run once (test)
hermes cron run job

# Enable/disable
hermes cron enable job
hermes cron disable job

# Remove
hermes cron delete job
```

## Example: Robust Cron Script Template

```bash
#!/bin/bash
# /path/to/robust-script.sh

set -euo pipefail

# Config
LOCKFILE="/var/lock/robust-script.lock"
LOGFILE="/var/log/robust-script.log"
ENVFILE="$HOME/.hermes/.env"

# Load env
if [[ -f "$ENVFILE" ]]; then
    set -a; source "$ENVFILE"; set +a
fi

# Lock
exec 9>"$LOCKFILE"
flock -n 9 || { echo "$(date): Already running" >> "$LOGFILE"; exit 1; }

# Log start
echo "$(date): Starting" >> "$LOGFILE"

# Main work (with error handling)
main() {
    local msg
    msg=$(fetch_data) || { send_alert "Fetch failed"; return 1; }
    send_message "$msg" >> "$LOGFILE" 2>&1
}

# Cleanup on exit
cleanup() {
    echo "$(date): Done" >> "$LOGFILE"
}
trap cleanup EXIT

main
```

## Iran-Specific: Jalali Calendar

```bash
# Install: sudo apt install jalali-calendar
# Or use python
python3 -c "
import jdatetime
print(jdatetime.datetime.now().strftime('%Y/%m/%d %H:%M:%S'))
"
```