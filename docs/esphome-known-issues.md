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

## I2C Bus API: `on_boot` Lambda schreiben

`IDFI2CBus` (`bus_int`) hat **kein `write_byte()` / `write_bytes()`** – die gibt es nur auf `I2CDevice`.

Für register-basierte I2C-Writes im Lambda (`on_boot`, etc.) muss `write(address, buffer, len)` mit kombiniertem Register+Value-Buffer verwendet werden:

```cpp
const uint8_t regval[] = {0xFE, 0xFF};  // [register, value]
bus_int->write(0x15, regval, 2);
```

Das `only_generate` erkennt diesen Fehler nicht – der C++ Code wird syntaktisch korrekt generiert, scheitert aber beim eigentlichen Compile (exit 1, leere Ausgabe).

**WARNUNG:** Auch ein korrekter I2C-Write auf den CST816S Touchcontroller (Adresse 0x15) im `on_boot`-Lambda kann das Display zum Absturz bringen (Blank-Display nach OTA). Daher: I2C-Writes auf Touchcontroller-Adressen im `on_boot` vermeiden.

## Compile-Troubleshooting: Leere Ausgabe trotz Fehler

Wenn der Full-Compile mit exit 1 (oder 2) und völlig leerer Ausgabe fehlschlägt:

1. **`/json-config` prüfen** – liefert detaillierte Fehler (YAML, C++ Lambda Syntax)
2. **`/compile` mit `only_generate`** – exit 0 = YAML + Code-Generierung OK
3. **Wenn `only_generate` exit 0, Full-Compile exit 1** → C++ Compile-Fehler (z.B. falsche API-Aufrufe in Lambdas). Die Fehlermeldungen erscheinen nicht im WebSocket-Output.
4. **Minimal-Test:** Lambda durch `ESP_LOGI("tag", "msg")` ersetzen → wenn Compile dann klappt, liegt es am Lambda-Code.

## Display-Update blockiert Touch-Responsiveness

Bei Displays mit `update_interval` (z.B. 200ms) blockiert `id(display).update()` im `on_touch`-Lambda die Touch-Verarbeitung:

- **Problem:** Jeder Touch triggert ein full Redraw (50-100ms bei QSPI 360×360). Währenddessen werden neue Touch-Interrupts nicht verarbeitet → schnelle Taps (Lautstärke) gehen verloren.
- **Lösung (theoretisch):** `id(display).update()` aus `on_touch` entfernen. Die Aktions-Skripte haben bereits `component.update: display`. Das `update_interval` fängt den Rest.
- **Problem in der Praxis:** Zusammen mit dem `on_boot`-Lambda zum CST816S Autosleep-Deaktivieren führte das Entfernen von `id(main_display).update()` zu einem Blank-Display. Nach Revert beider Änderungen funktioniert das Display wieder.
- **Aktueller Stand:** `id(main_display).update()` ist wieder im `on_touch` enthalten. Touch-Responsiveness ist suboptimal, aber Display-Funktion hat Priorität.

**Touch-Feedback:** `flash_counter` im Display-Lambda statt Force-Update – das Feedback kommt beim nächsten ohnehin fälligen Redraw.

## Touch: CST816S Autosleep deaktivieren

Der CST816S geht nach ~2s ohne Berührung in den Standby. Das verursacht:

1. **Setup-Failure beim Boot** – Chip antwortet nicht auf I2C → `skip_probe: true` nötig
2. **Verzögerung beim ersten Touch** – Chip muss aufwachen (~100-300ms)

**AUTOSLEEP-DEAKTIVIERUNG NICHT EMPFOHLEN** – führt zu Blank-Display:

Ein `on_boot`-Lambda, das per `bus_int->write(0x15, ...)` Register `0xFE = 0xFF` setzt, kann das Display komplett schwarz machen. Der I2C-Write auf den Touchcontroller während Boot stört vermutlich die Display-Initialisierung.

Stattdessen: `skip_probe: true` belassen und die ~100-300ms Wake-up-Latenz des ersten Touchs in Kauf nehmen. Das Display bleibt dann sichtbar.

**Nicht verwenden:**
```yaml
esphome:
  on_boot:
    priority: -100
    then:
      - lambda: |-
          const uint8_t regval[] = {0xFE, 0xFF};
          bus_int->write(0x15, regval, 2);
```

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
