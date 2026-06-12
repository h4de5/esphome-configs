# Waveshare ESP32-S3-Touch-LCD-1.85C

Board: **ESP32-S3-Touch-LCD-1.85C** (nicht die Basisversion ohne C)
Unterschied: **C-Version** hat **PCA9554 GPIO-Expander** + **PCM5101 Audio-DAC**.

## Spezifikation

| Komponente | Details |
|---|---|
| SoC | ESP32-S3, dual-core Xtensa LX7, 240MHz |
| PSRAM | 8MB octal, 80MHz |
| Flash | 16MB |
| Display | ST77916, QSPI, 360×360 round, `invert_colors: true` |
| Touch | CST816S, I2C (Adresse 0x15) |
| Audio | **PCM5101** Stereo-DAC (kein I2C!) + Power-Amplifier → **1x 8Ω 2W mono** |
| GPIO Expander | PCA9554, I2C (Adresse 0x24) |
| RTC | PCF8563 (nicht verwendet / nicht konfiguriert) |
| IMU | QMI8658 (nicht verwendet / nicht konfiguriert) |
| TF Card | SDMMC 4-bit |
| Mikrofon | I2S (nicht verwendet) |

## Pinout

### Display (ST77916, QSPI)

| Signal | GPIO |
|---|---|
| LCD_SDA0 (D0) | GPIO46 |
| LCD_SDA1 (D1) | GPIO45 |
| LCD_SDA2 (D2) | GPIO42 |
| LCD_SDA3 (D3) | GPIO41 |
| LCD_SCK | GPIO40 |
| LCD_CS | GPIO21 |
| LCD_BL | GPIO5 (PWM via LEDC) |
| LCD_RST | PCA9554 Pin **2** (nicht Pin 1!) |
| TP_RST | PCA9554 Pin **0** |

### Audio (PCM5101, I2S)

| Signal | GPIO |
|---|---|
| I2S_BCLK | GPIO48 |
| I2S_LRCK | GPIO38 |
| I2S_DOUT | GPIO47 |
| I2S_MCLK | **Nicht angeschlossen** – PCM5101 hat internen PLL |

Wichtig: PCM5101 braucht kein MCLK. `i2s_mclk_pin` darf nicht gesetzt werden.
`channel: left` für Mono (nur ein Kanal ist verstärkt).

### I2C (shared bus)

| Signal | GPIO |
|---|---|
| I2C_SCL | GPIO10 |
| I2C_SDA | GPIO11 |

Bus-Adressen: CST816S (0x15), PCA9554 (0x24), PCF8563 (0x51), QMI8658 (0x6B)

### TF Card (SDMMC 4-bit)

| Signal | GPIO |
|---|---|
| SD_CLK | GPIO15 |
| SD_CMD | GPIO14 |
| SD_D0 | GPIO16 |
| SD_D1 | GPIO17 |
| SD_D2 | GPIO12 |
| SD_D3 | GPIO13 |

### Touch (CST816S)

| Signal | GPIO |
|---|---|
| Touch_INT | GPIO4 (kein `allow_other_uses: true` – führt zu Config-Fehler) |

### Sonstiges

| Signal | GPIO |
|---|---|
| BOOT | GPIO0 |
| RTC_INT | GPIO6 |
| GPIO2 | MIC_WS (Mikrofon), nicht MCLK! |

## Audio Architektur

- PCM5101 Stereo-DAC (Hardware-Pins, kein I2C, keine audio_dac-Konfiguration)
- Power-Amplifier: verstärkt **einen** Kanal (left)
- Lautsprecher: 8Ω, 2W, mono
- `channel: left` in ESPHome Config
- Kein `i2s_mclk_pin` – PCM5101 hat internen PLL

## ESPHome Config Struktur

```yaml
substitutions:
  devicename: "esp32_s3_1"
  # ... Pin-Definitionen ...

<<: !include common/esp32-s3.yaml  # Basis: ESP-IDF, PSRAM, WiFi, OTA, etc.

status_led:
  pin: GPIO19  # GPIO19 = kein physischer Pin → blaue LED deaktiviert

# I2C für Touch + PCA9554
i2c:
  sda: GPIO11
  scl: GPIO10
  frequency: 400kHz

# PCA9554 GPIO Expander (für LCD_RST Pin 2, TP_RST Pin 0)
pca9554:
  - id: pca9554_device
    i2c_id: bus_int

# PCM5101 Audio (kein MCLK!)
i2s_audio:
  i2s_lrclk_pin: GPIO38
  i2s_bclk_pin: GPIO48

speaker:
  - platform: i2s_audio
    id: speaker_id
    dac_type: external
    i2s_dout_pin: GPIO47
    channel: left

# QSPI Display
spi:
  id: display_qspi
  type: quad
  clk_pin: GPIO40
  data_pins: [GPIO46, GPIO45, GPIO42, GPIO41]

display:
  - platform: qspi_dbi
    id: main_display
    model: CUSTOM
    rotation: 90°
    data_rate: 80MHz
    cs_pin: GPIO21
    dimensions: { height: 360, width: 360 }
    invert_colors: true
    reset_pin:
      pca9554: pca9554_device
      number: 2
    # ... init_sequence: Waveshare offiziell ...

# Touch
touchscreen:
  - platform: cst816
    interrupt_pin:
      number: GPIO4
    reset_pin:
      pca9554: pca9554_device
      number: 0
    i2c_id: bus_int
```

## Bekannte Fallstricke

1. **PCM5101 vs ES8311**: Das Board hat **PCM5101**, keinen I2S-Codec. Kein `audio_dac`, kein I2C für Audio.
2. **GPIO2 ist MIC_WS**, nicht MCLK. PCM5101 hat PLL → kein MCLK nötig.
3. **LCD_RST auf PCA9554 Pin 2** (1.85C Variante). Die 1.85 (non-C) hat LCD_RST auf PCA9554 Pin 1.
4. **`allow_other_uses: true`** ist bei Touch-Interrupt GPIO4 nicht erlaubt.
5. **QSPI DBI braucht `type: quad` + `data_pins` (Liste, 4 Pins)** – kein `interface: any` oder `dc_pin`.
6. **`rotation: 90°`** für Kabel rechts.
7. **`invert_colors: true`** notwendig für korrekte Farbdarstellung.
8. **`qspi_dbi`** ist deprecated in 2026.5.3, ersetzt durch `mipi_spi`.
9. **Bluetooth deaktivieren** falls nicht benötigt (spart Speicher).
10. **CST816S Standby**: Der Touch-Controller geht nach ~2s Inaktivität in Standby. `skip_probe: true` verhindert Setup-Failures.
    - **ACHTUNG – I2C Write kann Display zerstören**: Das Deaktivieren des Autosleep per `on_boot`-Lambda via `bus_int->write(0x15, ...)` kann nach OTA-Flash dazu führen, dass das Display komplett schwarz bleibt. Grund vermutlich I2C-Bus-Timing während Boot, das die Display-Initialisierung stört. Daher: **Kein I2C-Write an CST816S im `on_boot`** – lieber den ~100-300ms Wake-up auf ersten Touch in Kauf nehmen.
    - `skip_probe: true` allein ist ausreichend für stabilen Boot.
11. **I2C Bus write im on_boot Lambda**: `IDFI2CBus` hat kein `write_byte()` / `write_bytes()`. Stattdessen `bus_int->write(address, buffer, len)` mit kombiniertem Register+Value-Buffer.
    - **GEFAHR**: Diese Methode kann bei CST816S Adresse 0x15 das Display zum Absturz bringen (s. Punkt 10). Verwendung vermeiden.
12. **Display-Update im Touch-Handler**: Früher wurde `id(main_display).update()` aus `on_touch` entfernt, um schnelle Taps nicht zu blockieren. Nach Revert (Display war blank nach `on_boot`-Lambda) ist `id(main_display).update()` wieder im `on_touch` enthalten. Der Trade-Off: Touch-Responsiveness vs. Display-Sichtbarkeit – aktuell priorisieren wir sichtbares Display.

## Display Lambda Round-Layout

Display ist 360×360, Zentrum bei (180,180). Positionen müssen innerhalb des Radius 180 bleiben:

```
y:35-85   → Lautstärke (- / +)
y:110     → Media Metadata (Titel/Künstler)
y:140     → Uhrzeit (groß)
y:195     → Wochentag (ausgeschrieben, Deutsch)
y:225     → Datum + "Rd" Button rechts
y:285     → Transport (|<  >/||  >|)
```

Touch-Zonen sind entsprechend definiert – `id(main_display).update()` nach jeder Aktion.

## Naming Convention

Device `esp32_s3_1`, friendly name in HA per user gesetzt. Alle HA Entities folgen `{domain}.esp32_s3_1_{name}`:

| `name:` | HA Entity | Beschreibung |
|---|---|---|
| `"Target Playername"` | `text.esp32_s3_1_target_playername` | Ziel-Player (Entity-ID) |
| `"Player State"` | `text.esp32_s3_1_player_state` | State des Ziel-Players |
| `"Title"` | `text.esp32_s3_1_title` | Media Title |
| `"Artist"` | `text.esp32_s3_1_artist` | Media Artist |
| `"Volume"` | `sensor.esp32_s3_1_volume` | Volume Level |
| `"Player"` | `media_player.esp32_s3_1_player` | Sendspin Player |
| `"Speaker"` | `media_player.esp32_s3_1_speaker` | Lokaler Speaker |
| `"Display Backlight"` | `light.esp32_s3_1_display_backlight` | Backlight |
| `"Volume"` | `number.esp32_s3_1_volume` | Lautstärke-Regler |
| `"Display Sleep Timer"` | `number.esp32_s3_1_display_sleep_timer` | Sleep Timer |
| `"Alarm"` | `switch.esp32_s3_1_alarm` | Wecker Automation |
| `"Start Music"` | `button.esp32_s3_1_start_music` | Zufallswiedergabe |
| `"Stop"` | `button.esp32_s3_1_stop` | Musik Stop |
| `"Restart"` | `button.esp32_s3_1_restart` | Restart |

Gleiches Schema für `esp32_s3_2` etc.

## Referenzen

- Offizielle Waveshare Wiki: https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85
- ulsmith Reference Config: https://github.com/ulsmith/home-assistant-esphome-esp32-s3-touch-lcd-185c
- ESPHome QSPI DBI: https://esphome.io/components/display/qspi_dbi.html
