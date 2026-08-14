# Nginx Proxy for Chat UI at `/chat/` path

## Context
Built a FastAPI chat backend on port 8001, served via Nginx at `domain.com/chat/` for Mahra (AI agent) UI.

## Nginx Config (added to `/etc/nginx/sites-enabled/resin`)

```nginx
server {
    listen 80;
    server_name u2ssqxd25095.dxdx5.com;

    location / {
        proxy_pass http://127.0.0.1:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    }

    # --- Chat UI proxy (NEW) ---
    location /chat/ {
        proxy_pass http://127.0.0.1:8001/;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_read_timeout 86400;
    }

    location /static/ {
        alias /var/www/resin-static/;
    }

    location /media/ {
        alias /var/www/resin-media/;
    }
}
```

## Key Points
- **Trailing slash in proxy_pass**: `proxy_pass http://127.0.0.1:8001/;` (with trailing slash) strips `/chat/` prefix before forwarding to backend
- **WebSocket headers**: `Upgrade` and `Connection "upgrade"` headers enable WebSocket support (FastAPI uses this for real-time features)
- **Long timeout**: `proxy_read_timeout 86400` prevents timeout for long-running AI responses
- **Test locally first**: `curl http://localhost:8001/` should return 200 before testing via Nginx

## Backend Port Conflict
- **Django + Gunicorn runs on 8000** — default for Django
- **Chat backend on 8001** — separate port to avoid conflict
- **Nginx routes**:
  - `/` → 8000 (Django)
  - `/chat/` → 8001 (Chat UI)

## FastAPI Backend
Simple HTTP server (no frameworks) on port 8001:
```python
# /tmp/chat_backend.py
import http.server, json, http.client
# ... serves /chat/ HTML and /chat/api/chat POST endpoint
```

## Verification Commands
```bash
# Test backend directly
curl -X POST http://localhost:8001/api/chat -H "Content-Type: application/json" -d '{"message":"سلام"}'

# Test via Nginx
curl -X POST http://localhost/chat/api/chat -H "Content-Type: application/json" -d '{"message":"سلام"}'

# Test UI page
curl http://localhost/chat/
```

## Reload After Changes
```bash
nginx -t && systemctl reload nginx
```

## Common Issues
| Symptom | Cause | Fix |
|---------|-------|-----|
| 404 on `/chat/` | Wrong Nginx config file | Edit `/etc/nginx/sites-enabled/resin` (NOT `sites-available/default`) |
| 404 on `/chat/api/chat` | Missing trailing slash in proxy_pass | Add `/` at end: `proxy_pass http://127.0.0.1:8001/;` |
| WebSocket errors | Missing upgrade headers | Add `proxy_http_version 1.1; proxy_set_header Upgrade $http_upgrade; proxy_set_header Connection "upgrade";` |
| 404 from backend | Path not stripped | Ensure trailing slash in `proxy_pass http://127.0.0.1:8001/;` |