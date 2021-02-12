# Gaming room illumination with CS:GO game state integration on NodeMCU ESP32 and WS2812B LED strip
If you're dreaming of a low-cost gaming room illumination which also shows some effects while you're playing Counter-Strike: Global Offensive, this might be your starting block :)

## Features
- CS:GO: Health points shown at first 40 LEDs of the strip
  - 100 HP = green, 50 HP = yellow, 1 HP = red
- CS:GO: Armor points shown at last 20 LEDs of the strip
  - 100 AP = blue full brightness, 50 AP = blue moderate brightness
- CS:GO: When bomb planted, elapsed seconds are shown as a red progress bar on the first 40 LEDs
- Background illumination takes place and slowly sweeps through random colors when CS:GO is not active (default inactivity time is 2 minutes)

## Required hardware
- ESP32 (I have "NodeMCU ESP32" development board with a Chinese quality and Chinese price of 5€)
- 1 m long 5-volt WS2812B led strip with 60 LEDs (I had one from eBay for 7€. Search for "WS2812B led strip 5V")
- 5 V DC power source with at least 2.7 A output current (P = UI = 5 V * 2.7 A = 13.5 W)
- A couple of jumper wires
- Micro-USB cable for programming the ESP32

## Setup using [Visual Studio Code](https://code.visualstudio.com/)
- Clone this repository (Ctrl+Shift+P, type "git:clone", https://github.com/epetin/gaming-lights-csgo.git)
- Install PlatformIO IDE extension (Ctrl+Shift+X, search for platformio)
- At PlatformIO Home select "Import Arduino Project"
  - Choose board: NodeMCU-32S
- Install required libraries and add them to the project (PlatformIO->Libraries->Search libraries...)
  - fastled/FastLED@^3.4.0
  - ottowinter/ESPAsyncWebServer-esphome@^1.2.7
  - bblanchon/ArduinoJson@^6.17.2
- Set your WiFi SSID and password in [user_config.h](include/user_config.h)
- Connect ESP32 to the PC with micro-USB cable
- Hit "PlatformIO: Upload" at the bottom of the VS Code window
- Press and hold "BOOT" button next to the USB connector on ESP32 until you can see that the binary is being uploaded to the device
- Open serial monitor, "PlatformIO: Serial Monitor") at the bottom of the VS Code window
  - You should see IP address of the ESP32 if it connected to your WiFi successfully
- Set this IP address in [gamestate_integration_esp32.cfg](csgo_cfg/gamestate_integration_esp32.cfg) and copy file to the CS:GO cfg directory (located at SteamLibrary\steamapps\common\Counter-Strike Global Offensive\csgo\cfg)

## What next?
Tweak with the parameters in [user_config.h](include/user_config.h) and let me know if you have any cool improvement / development ideas on this project :)