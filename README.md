# Gaming room illumination with CS:GO game state integration on NodeMCU ESP32 and WS2812B LED strip
If you're dreaming of a low-cost (~20 €) gaming room illumination which also shows some effects while you're playing Counter-Strike: Global Offensive, this might be worth reading :)

## Features
- Idle illumination takes place at startup and after CS:GO inactivity. Idle lighting slowly sweeps through random colors. Inactivity time (default 2 min) and speed of the color sweep are controlled by corresponding values in [user_config.h](include/user_config.h)
- CS:GO: Health points are illustrated in the first 40 LEDs of the strip
  - Adjustable linear color scale from green to red. 100 HP = green, 50 HP = yellow, 1 HP = red.
- CS:GO: Armor points are illustrated in the last 20 LEDs of the strip
  - Linear brightness scale. 100 AP = blue full brightness, 50 AP = blue moderate brightness, etc.
- CS:GO: When bomb planted, elapsed seconds are shown as a red progress bar on the first 40 LEDs: one LED equals one second

## Required hardware
- ESP32 (maybe ESP8266 would also work out of the box)
  - I happened to have "NodeMCU ESP32" development board with a Chinese price of 5 €
- 1 m long 5-volt WS2812B led strip with 60 LEDs
  - from an online marketplace for 7 €
  - Search for "WS2812B led strip 5V"
  - Also other LED strip lengths (= LED counts) will work, as long as [user_config.h](include/user_config.h) is updated
- 5 V DC power source with at least (60 mA * \<total amount of leds\>) output current.
  - For example, with 60 LEDs this would be 3.6 A, which equals to required output power of P = UI = 5 V * 3.6 A = 18 W.
  - From an online marketplace for 3-4 €
  - Search for "5V 4A DC power supply"
- Power plug jack for easy powering of the LED strip
  - Search for "2.1x5.5mm DC Power Plug Jack"
- A couple of jumper wires (male-to-female)
- Micro-USB cable for programming the ESP32

## Wiring
- ESP32 GND#1 to WS2812B GND
- ESP32 5V to WS2812B 5V
- ESP32 pin 15 to WS2812B DIN (middle)
- ESP32 GND#2 to power source GND
- Power source GND to WS2812B external power source GND
- Power source 5V to WS2812B external power source 5V

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

## Preview
### Explode that fountain
<img src="gifs/explosion.gif" alt="Bomb explodes" width="800">
<br /><br />

### Noob gets killed by a bot
<img src="gifs/killed_by_bot.gif" alt="Noob killed by a bot" width="800">
<br /><br />

### Idle state with 200x speed
<img src="gifs/idle_200x.gif" alt="Idle state with 200x speed" width="800">
<br /><br />

## What next?
Tweak with the parameters in [user_config.h](include/user_config.h) and let me know if you have any cool improvement / development ideas on this project :)