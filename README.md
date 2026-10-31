# Bitox MiniBot

A compact ESP32-based interactive desk companion built around a 128x64 OLED display. This project combines a friendly animated face, live weather data, a clock, a stopwatch/timer, and small mini-games into a single battery-powered desktop device.

The main firmware is implemented in [Bitox/Main/Main.ino](Bitox/Main/Main.ino), and the hardware reference for the charging board is available in [Bitox/Circuit/Tp4056-5V-1A-18650-Type-C-USB-Lithium-Battery-Charging-Board-Charger-Module-with-Protection.avif](Bitox/Circuit/Tp4056-5V-1A-18650-Type-C-USB-Lithium-Battery-Charging-Board-Charger-Module-with-Protection.avif).

## Project Highlights

- ESP32-powered smart display with animated face expressions
- Live weather and forecast using OpenWeatherMap
- Multiple clock and weather display styles
- Stopwatch and countdown timer functions
- Tap, double-tap, and long-press interaction model
- Auto mode for rotating display states
- Small game mode for quick interaction
- Settings stored in flash memory for persistence

## Hardware Overview

This build is designed around an ESP32 microcontroller paired with:

- 128x64 SSD1306 OLED display
- I2C communication bus
- One tactile pushbutton for interaction
- TP4056-based Li-ion charging module for 18650 cell charging
- USB-C or USB power input with battery protection

## Repository Structure

- [Bitox/Main/Main.ino](Bitox/Main/Main.ino) — main firmware sketch
- [Bitox/Circuit](Bitox/Circuit) — hardware reference image and board assets

## Required Software

Before uploading the sketch, install the following in the Arduino IDE:

- Arduino IDE or VS Code with Arduino tooling
- ESP32 board package
- Adafruit SSD1306 library
- Adafruit GFX library
- Arduino_JSON library

## Wiring / Pin Configuration

The code is configured for the following hardware pins:

| Function | Pin |
| --- | --- |
| OLED SDA | 41 |
| OLED SCL | 42 |
| Button | 7 |

The display communicates over I2C, and the button uses an internal pull-up configuration.

## Configuration

Open [Bitox/Main/Main.ino](Bitox/Main/Main.ino) and update the Wi-Fi and weather credentials near the top of the file:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* OPENWEATHER_API_KEY = "YOUR_OPENWEATHER_API_KEY";
const char* CITY = "YOUR_CITY";
const char* COUNTRY_CODE = "US";
const char* TIMEZONE = "IST-6:00";
```

Important:

- Replace the sample credentials with your own before compiling.
- Use a valid OpenWeatherMap API key for weather and forecast data.
- If Wi-Fi is unavailable, the device will still boot but some online features will be limited.

## Upload Procedure

1. Connect the ESP32 board to your computer.
2. Open [Bitox/Main/Main.ino](Bitox/Main/Main.ino) in the Arduino IDE.
3. Select the correct ESP32 board and COM port.
4. Install the required libraries if prompted.
5. Compile and upload the sketch.
6. Power the board and verify the boot animation and Wi-Fi connection sequence.

## Controls

The project uses a simple pushbutton interaction model:

- Single tap: cycle or trigger current mode action
- Double tap: confirm selection or trigger a secondary action
- Long press: open menu or close menu

### Modes

The device supports these primary modes:

- Clock faces
- Weather display
- Animated scenes
- Face expressions
- Mini-game
- Auto mode
- Timer / stopwatch

## Features in Practice

Once running, the device can:

- Show the current time in different visual styles
- Display weather and a short forecast for a configured city
- Change the face mood based on conditions or user interaction
- Present a friendly animated character on the OLED screen
- Run a small endless runner game using the same button
- Keep timing functions useful for desk productivity

## Notes

This project is intended for hobby, experimentation, and learning. It is especially suitable for:

- personal desk gadgets
- mood displays
- interactive weather dashboards
- embedded systems prototyping

The firmware is intentionally self-contained and can be adapted to a custom enclosure or battery-powered case.

## License

This project is provided as-is for educational and personal use. If you plan to distribute or commercialize it, please check the licensing requirements of the libraries and any third-party assets before publishing.

## Recommended Next Improvements

- Add a custom enclosure design for the ESP32 and OLED screen
- Improve battery charging and power management
- Add a low-power sleep mode for longer battery life
- Expand the animation system and expression library
- Add more weather layouts and richer visual themes

---

Created for the Bitox MiniBot project.
