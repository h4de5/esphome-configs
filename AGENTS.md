# Docker-ESPHome

Arbeitsverzeichnis: `/workspace/development/docker-esphome/`

ESPHome Konfigurationsverzeichnis für das Heimnetzwerk (192.168.0.0/24).

## Agent-Workflow

- YAML-Konfigurationen lesen/bearbeiten
- **Validierung** via `/json-config?configuration=<file>` (HTTP GET) – prüft NUR YAML-Syntax, keine C++ Lambda-Fehler!
- **Compile** via WebSocket `/compile` (full, ohne `only_generate`) – prüft YAML + C++ Lambda → exit 0 = alles OK
- **OTA Upload** via WebSocket `/upload` – nur Flashen, getrennt nach erfolgreichem Compile
- **Pflicht-Reihenfolge:**
  1. `compile` (full build, OHNE `only_generate`) → exit 0 abwarten
  2. `upload` (OTA flashen) → exit 0 abwarten
- `/run` (Compile + OTA in einem) vermeiden – getrennte Schritte geben klare Fehlerausgaben
- `/json-config` liefert detailliertere YAML-Fehler als `/compile` mit `only_generate`
- Status abfragen (online/offline via `/ping`)
- Änderungen an Configs nur nach Absprache mit User
- Keine Änderungen an `common/`-Dateien ohne explizite Aufforderung

## Deploy-Workflow (Python)

ESPHome Container: `http://192.168.0.6:6052/`

### Schritt 1: Full Compile (YAML + C++ Lambda testen)

```python
import json, websocket
ws = websocket.create_connection(
    'ws://192.168.0.6:6052/compile',
    timeout=600,
    suppress_origin=True  # Wichtig! sonst 403
)
ws.send(json.dumps({
    'type': 'spawn',
    'configuration': 'esp32_s3_1.yaml',
    # KEIN only_generate – full build!
}))
while True:
    msg = ws.recv()
    data = json.loads(msg)
    if data.get('event') == 'line':
        line = data.get('message', '')
        if line.strip():
            print(line)  # Fehler sichtbar machen
    elif data.get('event') == 'exit':
        print(f'Exit: {data.get("code")}')
        break
ws.close()
```

### Schritt 2: OTA Upload (nur nach exit 0 von Schritt 1)

```python
import json, websocket
ws = websocket.create_connection(
    'ws://192.168.0.6:6052/upload',
    timeout=600,
    suppress_origin=True
)
ws.send(json.dumps({
    'type': 'spawn',
    'configuration': 'esp32_s3_1.yaml',
    'port': 'OTA'
}))
while True:
    msg = ws.recv()
    data = json.loads(msg)
    if data.get('event') == 'line':
        line = data.get('message', '')
        if line.strip():
            print(line)
    elif data.get('event') == 'exit':
        print(f'Exit: {data.get("code")}')
        break
ws.close()
```

### Schnell-Variante (beide Schritte in einem Skript)

```python
import json, websocket
for ep in ['compile', 'upload']:
    ws = websocket.create_connection(
        f'ws://192.168.0.6:6052/{ep}',
        timeout=600,
        suppress_origin=True
    )
    d = {'type': 'spawn', 'configuration': 'esp32_s3_1.yaml'}
    if ep == 'upload':
        d['port'] = 'OTA'
    ws.send(json.dumps(d))
    while True:
        msg = ws.recv()
        data = json.loads(msg)
        if data.get('event') == 'line':
            line = data.get('message', '')
            if line.strip():
                print(f'[{ep}] {line}')
        elif data.get('event') == 'exit':
            print(f'[{ep}] Exit: {data.get("code")}')
            if data.get('code') != 0:
                print(f'{ep.upper()} FEHLGESCHLAGEN – Abbruch')
                exit(1)
            break
    ws.close()
```

## API Endpoints

| Endpoint                            | Methode | Beschreibung                                    |
| ----------------------------------- | ------- | ----------------------------------------------- |
| `/ping`                             | GET     | Online-Status (ICMP)                            |
| `/info?configuration=<file>`        | GET     | Config Metadaten                                |
| `/json-config?configuration=<file>` | GET     | Geparste Config als JSON (nur YAML-Prüfung!)    |
| `/edit?configuration=<file>`        | GET     | Config-Inhalt abrufen                           |
| `/edit?configuration=<file>`        | POST    | Config speichern                                |
| `/delete?configuration=<file>`      | POST    | Config löschen                                  |
| `/downloads?configuration=<file>`   | GET     | Firmware Download                               |
| `/compile`                          | WS      | Compile (mit `only_generate` = nur YAML-Check)  |
| `/upload`                           | WS      | OTA-Flash (kein Compile – vorher build nötig!)  |
| `/run`                              | WS      | Compile + OTA + Logs (nicht empfohlen, wenige Fehlerausgaben) |

## Wichtige Regeln fürs Deployen

1. **Immer erst `compile` (full), dann `upload`** – nie nur `/upload` allein
2. **Niemals `only_generate: True` für den finalen Build** – das prüft nur YAML, nicht C++
3. **`compile`-Ausgaben vollständig lesen** – C++-Fehler erscheinen in den `line`-Events
4. **Exit 0 ≠ garantiert funktionsfähig** – zur Sicherheit nach OTA das WebUI prüfen
5. **Gerät offline?** → Cold Start (Stecker ziehen, warten, einstecken) – normaler Reset reicht oft nicht

## Path-Mapping

Container `/config/` = Host `/workspace/development/docker-esphome/config/`

## Geräteübersicht

### ESP32

| Config            | Ort                | Funktion                                       |
| ----------------- | ------------------ | ---------------------------------------------- |
| `esp32_1.yaml`    | Büro               | DHT Temp/Feuchte                               |
| `esp32_2.yaml`    | Kinderzimmer David | RGB LED Strip 75                               |
| `esp32_3.yaml`    | Wohnzimmer         | RGB Christbaum + Dimmer                        |
| `esp32_4.yaml`    | Badezimmer         | DHT Temp/Feuchte                               |
| `esp32_5.yaml`    | Kinderzimmer Jakob | RGB LED Strip 108                              |
| `esp32_6.yaml`    | Wohnzimmer TV      | RGB Ambilight 116                              |
| `esp32_7.yaml`    | Büro               | Uhr/Mikro/Lautsprecher/Media                   |
| `esp32_8.yaml`    | Büro               | e-Ink 7.3" (ESP32-S3, ESP-IDF)                 |
| `esp32_9.yaml`    | Büro               | Uhr/Mikro/Lautsprecher/Voice                   |
| `esp32_10.yaml`   | Büro               | e-Ink 7.3" Online-Image (ESP32-S3)             |
| `esp32_s3_1.yaml` | -                  | **Sendspin Radio + Display** (Waveshare 1.85C), steuert externen Player `media_player.david_lautsprecher` |

### Gosund SP111 (ESP8285)

| Config                                          | Status     |
| ----------------------------------------------- | ---------- |
| `gosund_sp111_1.yaml` bis `gosund_sp111_6.yaml` | **Online** |
| `gosund_sp111_7.yaml` (Lichterkette)            | Offline    |
| `gosund_sp111_8.yaml`                           | **Online** |
| `gosund_sp111_9.yaml` (Keller Gefriertruhe)     | Offline    |
| `gosund_sp111_10.yaml` (Keller Hebeanlage)      | Offline    |

### Sonoff (ESP8266)

| Config                               | Status     |
| ------------------------------------ | ---------- |
| `sonoff_4chpro.yaml`                 | **Online** |
| `sonoff_d1.yaml`                     | **Online** |
| `sonoff_pow_r2.yaml` (Waschmaschine) | **Online** |

## opencode Konfiguration

- **Global**: `~root/.config/opencode/opencode.jsonc` – Ollama Provider, MCP Server, Agenten (architect, developer, tester, researcher)
- **Projekt**: `/workspace/development/docker-esphome/opencode.json` – Projektspezifische Einstellungen (Compaction, @qwen3.6 Subagent)
- Beide werden automatisch gemerged (Projekt überschreibt Global)

## Wichtige Dateien

- `*.yaml` - Geräte-Konfigurationen
- `common/` - Basis-Konfigurationen (per `!include` / packages)
- `config/secrets.yaml` - WLAN-Passwörter, API-Keys
- `assets/` - Fonts, Images
- `opencode.json` - opencode Config (Compaction deaktiviert)

## Detail-Dokumentation

- [Waveshare ESP32-S3-Touch-LCD-1.85C Board](docs/esp32-s3-touch-lcd-185c.md) – Pinout, Audio, Display, Touch, Fallstricke
- [Home Assistant Integration](docs/home-assistant.md) – ESPHome API, Scripts, Media Metadata, Entities
- [Music Assistant Integration](docs/music-assistant.md) – MA API, Sendspin, Playlists, Reverse Proxy
- [ESPHome Known Issues](docs/esphome-known-issues.md) – Compile-Probleme, Migration 2026.5.x
- [Critical Thinking Rules](docs/critical-thinking-rules.md) – Für den Agenten verbindlich

## Auto-Improve

Neue Erkenntnisse zu einem Thema werden **automatisch** in der entsprechenden `docs/`-Datei nachgetragen. Gibt es noch keine passende Datei, wird eine neue angelegt. Das gilt für:

- Hardware-Details (Pinout, Chips, Konfiguration)
- API-Endpunkte, Services, Skripte
- Fehlerbehebungen und Workarounds
- Entscheidungen des Users (z.B. "keine API-Tokens in Firmware")
- Architektur-Entscheidungen und Begründungen

Ziel: Die `docs/`-Dateien sind immer der aktuelle Wissensstand, ohne dass der User explizit daran erinnern muss.

## Subagent-Nutzung

Kleinere, abgeschlossene Aufgaben (Dateien lesen, Config validieren, Status checken, einfache Recherche) **sollten an den `@qwen3.6` Subagent delegiert werden**. Das spart Tokens des Hauptmodels und beschleunigt die Bearbeitung.

```yaml
# Verwendung via @qwen3.6 im Prompt
@qwen3.6  Lese die aktuelle Config von esp32_1.yaml und prüfe ob alle Pins korrekt sind
```

Der Agent läuft auf dem lokalen Ollama-Server (`http://192.168.0.6:11434/v1`) mit dem Model `qwen3.6:35b`.

## Wichtige Regeln

1. **Nicht als root compilieren** – Container läuft als User 99. `esphome` CLI ist nicht auf dem Host installiert – Validierung nur via Container-API (`/json-config` oder WebSocket `only_generate`).
2. **Keine API-Tokens in Firmware oder HA config** – User hat `http_request` und `rest_command` abgelehnt.
3. **`i2s_mclk_pin` niemals setzen** bei PCM5101 (interne PLL).
4. **`suppress_origin=True`** beim WebSocket-Compile (sonst 403).
5. **`common/`-Dateien nicht modifizieren** ohne explizite Aufforderung.
6. **`common/`-Dateien nicht modifizieren** ohne explizite Aufforderung.
7. **OAuth muss für externen Zugriff erhalten bleiben** – Conditional OAuth für LAN-IPs als Lösung.
