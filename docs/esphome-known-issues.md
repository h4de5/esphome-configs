# ESPHome Known Issues & Workarounds (2026.5.3)

## Compile-Probleme

### `i2s_audio` kompiliert nicht im Container
- Betrifft: Alle Configs mit `i2s_audio` (esp32_7, esp32_9, esp32_s3_1)
- Symptom: Exit 2, leere Ausgabe
- Grund: Fehlende Abhängigkeiten im Container, nicht die Config
- `only_generate` (Validierung) funktioniert trotzdem (exit 0)

### `media_player.i2s_audio` removed in 2026.5.3
**ALT (führt zu Compile-Fehlern):**
```yaml
media_player:
  - platform: i2s_audio
    name: "My Media Player"
    dac_type: external
    i2s_dout_pin: GPIO47
```

**NEU:**
```yaml
i2s_audio:
  i2s_lrclk_pin: GPIOXX
  i2s_bclk_pin: GPIOXX

speaker:
  - platform: i2s_audio
    id: speaker_id
    dac_type: external
    i2s_dout_pin: GPIOXX

media_player:
  - platform: speaker
    name: "My Media Player"
    announcement_pipeline:
      speaker: speaker_id
```

### `announcement_pipeline` required
Seit 2026.5.3 MUSS `announcement_pipeline` bei `media_player.speaker` gesetzt sein. Ohne gibt es Compile-Fehler.

```yaml
media_player:
  - platform: speaker
    name: "Name"
    announcement_pipeline:
      speaker: speaker_id
```

### `speaker_source` + `announcement_pipeline`
Bei `media_player.speaker_source` darf `announcement_pipeline` nicht ohne `sources:` verwendet werden (sonst Parse-Fehler).

### Keine doppelten top-level Keys
Wenn ein Package (via `<<: !include common/...`) bereits Keys wie `binary_sensor:`, `spi:`, `text_sensor:` definiert, darf die Device-Config diese nicht nochmal auf top-level setzen → sonst Compile-Error.

## Validierung via API

- `/json-config?configuration=<file>` (HTTP GET) – parst Config und liefert volles JSON oder Error
- WebSocket `only_generate` – compile-valide Prüfung (exit 0 = ok)
- `esphome config` CLI ist **nicht auf dem Host installiert** – Validierung läuft ausschließlich über die Container-API
- Für Debugging: `/json-config` liefert detailreichere Fehler als `only_generate`

## Path-Mapping

```
Container: /config/...
Host:      /workspace/development/docker-esphome/config/...
```

## Root-Compile-Verbot

- Container läuft als User 99
- Root-Compile erzeugt Dateien mit Owner root → Permission-Probleme
- `esphome` CLI ist nicht auf dem Host installiert – kein lokales `esphome config` möglich
- Compilen/flashen macht der User selbst (oder via Container-API)

## Firmware-Binaries

Liegen unter: `config/.esphome/build/<name>/` mit Unterverzeichnissen pro Plattform.

## Erstflash via USB

1. Board per USB-C an Rechner (z.B. Raspberry Pi mit ESPHome Container)
2. BOOT-Taste gedrückt halten, RESET kurz drücken, BOOT loslassen → Download-Mode
3. `esphome run config/esp32_s3_1.yaml --device /dev/ttyACM0`
4. Nach Erstflash sind OTA-Updates möglich

## Sendspin

- Verfügbar ab ESPHome 2026.3.0+
- Multi-Room Audio via WLAN
- Minimal-Config:

```yaml
sendspin:
  id: sendspin_hub

media_source:
  - platform: sendspin
    id: sendspin_source

media_player:
  - platform: speaker_source
    id: external_media_player
    name: "Media Player"
    media_pipeline:
      speaker: speaker_id
      sources:
        - sendspin_source
```

## Display API: Nicht-existente Methoden

Folgende Methoden **gibt es nicht** im ESPHome Display – `filled_rounded_rectangle`, `rounded_rectangle` sind Fantasie-Namen.
Einzige Rectangle-Methoden:
- `rectangle(x, y, w, h, color)` – Umrandung
- `filled_rectangle(x, y, w, h, color)` – gefüllt

`circle`/`filled_circle`/`line`/`print`/`strftime` sind dagegen valide.

## OTA-Deployment via WebSocket API

Nach erfolgreichem Build kann die Firmware per `/upload` WebSocket direkt OTA geflasht werden:

```python
import json, websocket
ws = websocket.create_connection(
    'ws://192.168.0.6:6052/upload',
    timeout=120,
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
        print(data.get('message', ''))
    elif data.get('event') == 'exit':
        print(f'Exit: {data.get("code")}')
        break
ws.close()
```

`port: "OTA"` flasht per Netzwerk (nutzt die in der Config hinterlegte OTA-Adresse).
Alternativ `/dev/ttyUSB0` für serielles Flashen am Container-Host.

## MDI Icons auf Displays

Der ESPHome Container hat kein Internet – Google Fonts / gfonts funktionieren nicht.
Stattdessen MDI TTF lokal bereitstellen und via `file:` einbinden:

```yaml
font:
  - file: "assets/materialdesignicons.ttf"
    id: font_mdi
    size: 22
    glyphs:
      - "\U000F0388"   # music-note-eighth
```

TTF herunterladen:
```
curl -sLo config/assets/materialdesignicons.ttf \
  https://github.com/Templarian/MaterialDesign-Webfont/raw/master/fonts/materialdesignicons-webfont.ttf
```

Codepoints auf https://pictogrammers.com/library/mdi/ finden.

## ESPHome 2026.5.x Migration Notes

- `qspi_dbi` existiert noch, ist aber deprecated → ersetzt durch `mipi_spi`
- `media_player.i2s_audio` removed → `speaker.i2s_audio` + `media_player.speaker`
- `i2s_audio` ist Top-Level-Singleton (keine Liste), `i2s_bclk_pin` ist optional

## Troubleshooting: OTA schlägt fehl / Ping zeigt offline trotz online Gerät

Wenn der Benutzer bestätigt, dass das Gerät online ist (WLAN verbunden, läuft), aber der ESPHome Container via `/ping` `false` meldet und `/upload` (OTA) mit Exit 1 fehlschlägt:

**Lösung:** Cold Start (Stecker ziehen, warten, wieder einstecken) – ein einfacher Reset (`STRG+R` / `restart` via API) reicht nicht. Erst nach einem vollständigen Stromzyklus meldet sich das Gerät wieder korrekt bei der Container-API an und OTA funktioniert.

## Template Number/Text: `optimistic` + `lambda` inkompatibel

Bei `number.template` und `text.template` können `optimistic: true` und ein `lambda` **nicht** gleichzeitig verwendet werden.

```yaml
# FALSCH – führt zu Compile-Fehler:
number:
  - platform: template
    id: sleep_timer
    optimistic: true
    restore_value: true
    initial_value: 60
    lambda: |-           # <-- Fehler: optimistic + lambda
      return id(sleep_timer).state;
    set_action:
      ...

# RICHTIG – lambda weglassen, Wert wird intern gespeichert:
number:
  - platform: template
    id: sleep_timer
    optimistic: true
    restore_value: true
    initial_value: 60
    set_action:
      ...
```

- Mit `optimistic: true` + `restore_value: true` wird der Wert intern gespeichert und nach Neustart wiederhergestellt – ein `lambda` zum Lesen ist nicht nötig.
- Gleiches gilt für `text.template`, `sensor.template` etc.
- Fehlererkennung: `/json-config?configuration=...` gibt einen klaren Hinweis (`optimistic cannot be used with lambda`), `/compile` mit `only_generate` exit 2 = leer.
