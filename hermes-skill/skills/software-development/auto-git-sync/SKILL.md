---
name: auto-git-sync
description: Auto-sync skills and memory to GitHub repository on changes.
---

# Auto Git Sync Skill

## Purpose
Automatically syncs skills and memory changes to the configured GitHub repository.

## Trigger
Runs on cron schedule to ensure local changes are pushed to remote.

## Script
```bash
#!/bin/bash
# Sync path
cd /root/hermes-personal-config
# Update files from source
rsync -av --delete /root/.hermes/skills/ ./skills/
rsync -av --delete /root/.hermes/memories/ ./memory/

# Push to git
git add .
if ! git diff-index --quiet HEAD --; then
    git commit -m "Auto-sync: $(date)"
    git push origin main
fi
```

## Usage
Scheduled via cronjob: `cronjob(action='create', schedule='every 1h', script='auto-git-sync.sh')`
