# Music Assistant Integration

## Server

| Detail | Wert |
|---|---|
| Server | `192.168.0.6:8095` |
| Web UI (extern) | `https://{domain_extern_music}` |
| API (REST) | `POST http://192.168.0.6:8095/api` |
| API (WS) | `ws://192.168.0.6:8095/ws` |
| API Auth | Bearer JWT-Token |

## API Usage

### REST API

```python
import requests

response = requests.post(
    "http://192.168.0.6:8095/api",
    json={
        "message_id": "test-1",
        "command": "player_queues/play_media",
        "args": {
            "queue_id": "up98a316c0e2f4",
            "media": "library://playlist/5",
            "enqueue": False
        }
    },
    headers={"Authorization": "Bearer <TOKEN>"}
)
```

### WebSocket API

```python
import json, websocket
ws = websocket.create_connection("ws://192.168.0.6:8095/ws")
ws.send(json.dumps({
    "message_id": "1",
    "command": "players/player",
    "args": {"player_id": "up98a316c0e2f4"}
}))
while True:
    msg = ws.recv()
    print(json.loads(msg))
```

## Wichtige IDs

| Objekt | ID |
|---|---|
| Player "David Wecker Sendspin" | `up98a316c0e2f4` |
| Queue (gleiche ID) | `up98a316c0e2f4` |
| Playlist "500 Random tracks" | `library://playlist/5` |

## HA Integration

### Voraussetzungen
- **"Exposed to Home Assistant"** muss **ON** sein (per-player in MA Einstellungen)
- Sonst funktioniert `music_assistant.play_media` nicht

### HA Script `script.musik_start_zufallslieder`

```yaml
musik_start_zufallslieder:
  alias: "Musik Start - Zufallslieder"
  fields:
    target_player:
      description: "Ziel-Player Entity-ID"
      example: "media_player.david_wecker"
      required: true
      selector:
        entity:
          domain: media_player
  sequence:
    - service: music_assistant.play_media
      data:
        media_id: "library://playlist/5"
        media_type: "playlist"
        enqueue: false
      target:
        entity_id: "{{ target_player }}"
```

Aufgerufen von ESPHome via `homeassistant.service`:

```yaml
script:
  - id: script_500_random
    then:
      - homeassistant.service:
          service: script.musik_start_zufallslieder
          data:
            target_player: media_player.david_wecker
```

## Reverse Proxy / Base URL Problem

**Problem:** MA Base URL = `https://{domain_extern_music}`. Proxy (NPM + OAuth2-Proxy) verlangt Authentifizierung. HA Integration verbindet via LAN-IP, MA antwortet mit Base URL, HA wechselt dorthin → OAuth → Fehler.

**Lösung 1: Conditional OAuth (empfohlen)**
Nginx Config so anpassen, dass `/api/`-Requests aus dem LAN (192.168.0.0/24) ohne OAuth durchgelassen werden:

```nginx
set $skip_auth 0;
if ($remote_addr ~ ^192\.168\.) {
    set $skip_auth 1;
}

location / {
    if ($skip_auth = 0) {
        auth_request /oauth2/auth;
        error_page 401 = /oauth2/sign_in;
        # ... OAuth headers ...
    }

    proxy_pass http://192.168.0.6:8095;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection $http_connection;
    # ...
}
```

Alternativ: Zwei location-Blöcke:
```nginx
location /api/ {
    proxy_pass http://192.168.0.6:8095;
    # kein auth_request
}

location / {
    auth_request /oauth2/auth;
    # ...
    proxy_pass http://192.168.0.6:8095;
}
```

**Lösung 2: Base URL auf interne IP ändern**
MA Settings → Core → Web Server → Base URL = `http://192.168.0.6:8095`
Dann spricht HA direkt ohne OAuth. Externer Zugriff via Proxy funktioniert trotzdem.

**Lösung 3: Split DNS**
`{domain_extern_music}` intern auf `192.168.0.6` auflösen (Pi-hole/AdGuard/Fritzbox).
Dann Base URL bleiben lassen, aber Zertifikat+Auth-Probleme bleiben.

## ESPHome Sendspin Config

```yaml
sendspin:
  id: sendspin_hub

media_source:
  - platform: sendspin
    id: sendspin_source

media_player:
  - platform: speaker_source
    name: "David Wecker"
    id: sendspin_player
    media_pipeline:
      speaker: speaker_id
      sources:
        - sendspin_source
  - platform: speaker
    id: local_media_player
    name: "David Wecker Lautsprecher"
    announcement_pipeline:
      speaker: speaker_id
```

Zwei Media-Player teilen sich `speaker_id`, können aber nicht gleichzeitig spielen.
- `sendspin_player` (`speaker_source`) → Sendspin Multi-Room
- `local_media_player` (`speaker`) → Direkte URL/DLNA Wiedergabe

## Wichtige Regeln

- **Keine API-Tokens in der Firmware oder in HA configuration.yaml speichern.**
- Der User hat sowohl `http_request` (Token in ESPHome) als auch `rest_command` (Token in HA config) abgelehnt.
- `music_assistant.play_media` in HA Scripts ist der einzig akzeptierte Weg.
