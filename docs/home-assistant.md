# Home Assistant Integration

## ESPHome API

Die ESPHome-Geräte verbinden sich via native API mit HA:

```yaml
# In common/base.yaml via packages:
api:
  encryption:
    key: !secret api_encryption_key
```

`api_encryption_key` in `config/secrets.yaml`.

Geräte werden automatisch in HA erkannt und konfiguriert (kein manuelles Hinzufügen nötig).

## HA Scripts

### `script.musik_start_zufallslieder`

Startet eine zufällige Playlist auf einem beliebigen MA-Player:

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

Aufruf von ESPHome (via `homeassistant.service`):

```yaml
script:
  - id: script_500_random
    then:
      - homeassistant.service:
          service: script.musik_start_zufallslieder
          data:
            target_player: media_player.david_wecker
```

## Media Metadata

ESPHome liest Titel/Künstler-Information via `homeassistant` text_sensor:

```yaml
text_sensor:
  - platform: homeassistant
    entity_id: media_player.david_wecker_lautsprecher
    attribute: media_title
    id: ha_title
  - platform: homeassistant
    entity_id: media_player.david_wecker_lautsprecher
    attribute: media_artist
    id: ha_artist
```

Wird im Display-Lambda angezeigt:

```cpp
std::string title = id(ha_title).state;
std::string artist = id(ha_artist).state;
```

## Zeit-Synchronisation

ESPHome bezieht Zeit von HA:

```yaml
time:
  - platform: homeassistant
    id: esptime
    on_time_sync:
      then:
        - component.update: main_display
```

## Music Assistant Integration

Siehe [music-assistant.md](music-assistant.md) für Details:
- MA API (REST + WebSocket)
- Sendspin Multi-Room Konfiguration
- Playlist-IDs (Player `up98a316c0e2f4`, Playlist `library://playlist/5`)
- Wichtige Regel: "Exposed to Home Assistant" muss ON sein

## Reverse Proxy / OAuth Problem

Siehe [music-assistant.md](music-assistant.md#reverse-proxy--base-url-problem) für die Lösungsansätze:
- Conditional OAuth (LAN-Bypass im Nginx Proxy Manager)
- MA Base URL auf interne IP ändern
- Split DNS

**Wichtig:** OAuth muss für externen Zugriff erhalten bleiben. Conditional OAuth (nur LAN-IPs bypassen) ist die bevorzugte Lösung. Die Nginx Config muss noch getestet werden (User berichtete Fehler beim Einrichten).

## Bekannte Entities

Die ESPHome-Config `esp32_s3_1.yaml` registriert folgende HA-Entities:

| Entity ID | Typ | Beschreibung |
|---|---|---|
| `media_player.david_wecker` | media_player | Sendspin Multi-Room (speaker_source) |
| `media_player.david_wecker_lautsprecher` | media_player | Direkte Wiedergabe (speaker) |
| `light.display_backlight` | light | Display-Hintergrundbeleuchtung |
| `button.restart` | button | ESP32 Neustart |
| `text_sensor.*_ha_title` | text_sensor | Aktueller Musiktitel |
| `text_sensor.*_ha_artist` | text_sensor | Aktueller Interpret |
