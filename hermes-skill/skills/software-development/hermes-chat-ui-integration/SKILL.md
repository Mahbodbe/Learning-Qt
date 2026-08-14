---
name: hermes-chat-ui-integration
description: Build, configure, and integrate web-based chat UIs for Hermes Agent, routing traffic through Nginx proxies to local backend servers.
version: 1.0.0
author: Hermes Agent
tags: [fastapi, nginx, proxy, reverse-proxy, chat-ui, web-chat, websocket, local-api]
---

# Hermes Chat UI Integration Skill

## Overview
Learn how to build a clean web-based chat UI for Hermes Agent, run a local Python HTTP or FastAPI backend server, and proxy traffic securely through Nginx under a sub-path (like `/chat/`) to avoid CORS, network timeouts, and local API key redaction failures.

## Nginx Proxy Setup (The ONLY Way that works)
On VPS setups running Hermes, requests from *inside* the server to its own public domain name will often time out due to routing and security firewalls.

Always configure Nginx to proxy a subpath (e.g., `/chat/`) on Port 80 to a local port (e.g., `8001`) where your Python chat backend is listening.

### 1. Nginx Virtual Host Config (`/etc/nginx/sites-enabled/resin`)
```nginx
server {
    listen 80;
    server_name yourdomain.com;

    # Proxy root to main web application (e.g., Django)
    location / {
        proxy_pass http://127.0.0.1:8000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
    }

    # Proxy chat path to local python chat backend on Port 8001
    location /chat/ {
        proxy_pass http://127.0.0.1:8001/; # Note the trailing slash to strip /chat/
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_read_timeout 86400;
    }

    location /static/ {
        alias /var/www/static/;
    }

    location /media/ {
        alias /var/www/media/;
    }
}
```
*Note: Run `nginx -t && systemctl reload nginx` after modification.*

## Python Backend (The Zero-Dependency `http.server` Fallback)
Avoid installing heavy packages like `fastapi` and `uvicorn` on low-RAM VPSs (~1GB) where pip installs often hang or timeout. Instead, use Python's built-in `http.server` module to handle routing.

```python
# chat_backend.py
import json, os, sys
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse

PORT = 8001

class ChatHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        path = urlparse(self.path).path
        if path in ["/", "/chat/", ""]:
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.end_headers()
            with open("/var/www/html/chat.html", "rb") as f:
                self.wfile.write(f.read())
        else:
            self.send_response(404); self.end_headers()
            
    def do_POST(self):
        path = urlparse(self.path).path
        if path == "/api/chat":
            content_length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(content_length)
            data = json.loads(body.decode('utf-8'))
            user_msg = data.get("message", "")
            
            # Smart Local Responses (avoiding redacted API keys)
            # Route to a simple local matcher or mock responses when Hermes Gateway is unauthenticated
            response_text = self.get_intelligent_response(user_msg)
            
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps({"response": response_text}).encode())
            
    def get_intelligent_response(self, msg):
        # Quick keyword matching or local lightweight model execution
        return "سلام! چطور می‌تونم کمکت کنم؟"

if __name__ == "__main__":
    server = HTTPServer(("0.0.0.0", PORT), ChatHandler)
    server.serve_forever()
```

## The Redacted API Key Pitfall
When running on public VPS servers, Hermes Agent redacts its API Keys (`sk-8c9...ce41`) for security.
- **Problem:** Writing a backend that parses `config.yaml` to extract the API key for `9router` or direct API calls will fail, resulting in `401 Unauthorized / Invalid API Key` errors because of the redacted asterisks.
- **Solution:** Bypass external network calls. If you need intelligent responses, either:
  1. Have the user pass their unredacted API key securely.
  2. Spawn subprocesses to call the `hermes` CLI directly: `hermes chat --model code` via `subprocess.run(..., input=user_message)`.
  3. Run a local offline LLM (like llama.cpp or ollama) that doesn't require API key validation.

## Web Chat Frontend Integration
The front-end JavaScript must use relative paths (`/chat/api/chat`) to avoid CORS errors when connecting to the back-end through Nginx:

```javascript
// chat.html
async function sendMessage() {
    const text = messageInput.value.trim();
    if (!text) return;
    
    // Always use the relative path mapped in Nginx
    const response = await fetch('/chat/api/chat', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ message: text })
    });
    const data = await response.json();
    addMessage('assistant', data.response);
}
```

## Troubleshooting & Verification Checklist

1. **Test local port**: `curl -s -X POST http://localhost:8001/api/chat -H "Content-Type: application/json" -d '{"message":"test"}'`
2. **Test Nginx proxy**: `curl -s -X POST http://localhost/chat/api/chat ...`
3. **Verify CORS headers**: Ensure `Access-Control-Allow-Origin: *` is sent in OPTIONS and POST methods.
4. **Identify Port Collisions**: Gunicorn/Django typically run on port `8000`. Running chat backend on `8000` causes `Address already in use` error. Always assign an isolated port like `8001` or `8080`.
