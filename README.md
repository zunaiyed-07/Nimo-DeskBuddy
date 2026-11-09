# NIMO MiniBot

NIMO is an interactive desk companion built with an ESP32-C3 Super Mini and a
128x64 SSD1306 OLED. It includes animated faces and scenes, clock and weather
screens, a stopwatch/countdown timer, and a small button-controlled game.

## Hardware

- ESP32-C3 Super Mini
- 128x64 SSD1306 I2C OLED (typically address `0x3C`)
- Digital-output touch sensor module (for example, TTP223)
- Optional battery and charging circuit

### Wiring

| Component | ESP32-C3 Super Mini |
| --- | --- |
| OLED VCC | 3V3 |
| OLED GND | GND |
| OLED SDA | GPIO 5 |
| OLED SCL | GPIO 6 |
| Touch sensor OUT | GPIO 7 |
| Touch sensor VCC | 3V3 |
| Touch sensor GND | GND |

The firmware expects the sensor's digital output to go HIGH when touched. For
an active-low sensor, change `TOUCH_ACTIVE_LEVEL` near the top of
[Main/Main.ino](Main/Main.ino) to `LOW`. GPIO 5 and GPIO 6 are selected for I2C
to avoid the Super Mini's boot button (GPIO 9) and onboard LED (usually GPIO 8).
If your board revision labels pins differently, use the GPIO numbers printed
in its pinout and update `SDA_PIN`, `SCL_PIN`, or `BUTTON_PIN` near the top of
[Main/Main.ino](Main/Main.ino) to match your wiring. Power the OLED from 3.3 V
unless its manufacturer explicitly supports another supply voltage.

The firmware debounces touch presses for 40 ms and releases for 35 ms, which
filters brief electrical noise while allowing quick taps. If the sensor output
stays active while a finger is only nearby, reduce the sensor's sensitivity or
adjust its sensing pad according to the module's documentation; software cannot
distinguish that sustained signal from an actual touch.

## Arduino IDE setup

1. Install the Espressif **esp32** board package using Boards Manager.
2. Select **ESP32C3 Dev Module** (the generic ESP32-C3 target for the Super
   Mini) and the board's serial port.
3. If the board options are available, enable **USB CDC On Boot** when using
   the built-in USB connection.
4. Install these libraries from Library Manager:
   - Adafruit SSD1306
   - Adafruit GFX Library
   - Arduino_JSON
5. Open [Main/Main.ino](Main/Main.ino), compile, and upload.

`WiFi`, `WebServer`, `Preferences`, `HTTPClient`, and `Wire` are provided by the
ESP32 Arduino core.

## Configuration

Edit the configuration constants near the top of [Main/Main.ino](Main/Main.ino)
before uploading:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* OPENWEATHER_API_KEY = "YOUR_OPENWEATHER_API_KEY";
const char* CITY = "Sirajganj";
const char* COUNTRY_CODE = "BD";
const char* TIMEZONE = "BDT-6";
```

Use your 2.4 GHz Wi-Fi credentials and an OpenWeatherMap API key. Change the
city, ISO country code, and POSIX timezone string for your location. The
firmware will still start if Wi-Fi is unavailable, but time synchronization and
online weather data will not work. Do not commit personal Wi-Fi credentials or
API keys to the repository.

## Controls

- Single tap: advance the current selection or perform its action
- Double tap: select a menu item or perform the mode's secondary action
- Long press: open or close the menu

Available modes include clock faces, weather, animations, expressions, the
Dino game, auto mode, and a stopwatch/countdown timer.

- Double tap is recognized when the second touch begins within 550 ms of the
  first tap, so a normal second-touch release does not have to finish inside
  the interval.

## Phone dashboard

The device always starts its setup access point, so you can open the dashboard
to configure Wi-Fi even while it connects to a router. Connect your phone to
the setup network and open the address shown on the OLED:

- Wi-Fi name: `NIMO`
- Password: `nimo-setup-24` (change `SETUP_AP_PASSWORD` before uploading)
- Dashboard: `http://192.168.1.4`

In Device settings, scan for nearby Wi-Fi networks, choose one, enter its
password, and select **Connect and save**. NIMO keeps the dashboard available
on its setup network while connecting. A network is saved only after a
successful connection; saved credentials are retried automatically on later
boots and while the network is out of range. The compiled Wi-Fi settings are
used as the initial connection until a network is saved from the dashboard.
After connecting to the home Wi-Fi, the OLED briefly shows the device's home
network IP address; enter that address in a browser on the same Wi-Fi to reopen
the dashboard. The dashboard connection card also switches to this address
while connected.

On the Overview page, choose **Clock**, **Weather**, **Animation**, or
**Expression** to reveal the available choices. Choose **Timer** for a
phone-controlled stopwatch or countdown (1–60 minutes), including start,
pause, and reset controls. Choose **Dino game** to play on the OLED; use the
dashboard's Jump button to clear cacti and Restart game after a collision.
The firmware includes 24 animation scenes and 30 facial expressions, including
orbiting stars, a petal breeze, curious, grateful, and playful faces. The
dashboard also groups animations and expressions by collection under
Appearance.

The date and time can be entered manually in the configured device timezone.
Without internet time synchronization or a battery-backed RTC, the clock keeps
time only while the device remains powered; set it again after a power cycle.
This firmware uses Wi-Fi and does not provide Bluetooth control. Keep the
dashboard on a trusted network and do not expose its HTTP port to the internet.

## Repository contents

- [Main/Main.ino](Main/Main.ino) — firmware sketch
- `Circuit/PCB main.zip` — circuit/PCB reference files
