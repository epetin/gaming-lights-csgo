/******************************************************************************
 * GAMING-LIGHTS-CSGO
 * 
 * GAMING ROOM AND CS:GO BACKGROUND LIGHTING WITH ESP-32 AND WS2812B LED STRIP
 * ***************************************************************************/
#include <FastLED.h>
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "user_config.h"

/******************************************************************************
 * INTERNAL DEFINITIONS AND VARIABLES
 * ***************************************************************************/
#define BOMB_NOT_PLANTED 0
#define BOMB_PLANTED 1
#define BOMB_EXPLODED 2
#define BOMB_DEFUSED 3
#define BYTE_MAX 255
/* Two main colors in idle state lighting */
#define COLOR_RG 0
#define COLOR_GB 1
#define COLOR_BR 2

/* Web server */
AsyncWebServer server(ESP_WEB_SERVER_PORT);

/* LED control */
CRGB leds[NUM_LEDS_TOTAL];

/* Timer variables for idle lighting and bomb counter */
static unsigned long last_s_ms = 0;
static unsigned long now_ms = 0;
static unsigned long inactivity_ms = 0;
static unsigned long idle_led_ms = 0;

/* WiFi */
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

/* Idle state LED color control */
uint8_t idle_led_r = random(0, BYTE_MAX + 1);
uint8_t idle_led_g = random(0, BYTE_MAX + 1);
uint8_t idle_led_b = random(0, BYTE_MAX + 1);
uint8_t t_idle_led_r = 0;
uint8_t t_idle_led_g = 0;
uint8_t t_idle_led_b = 0;

/* Game data */
String player;
String bomb_state;
uint8_t bomb = BOMB_NOT_PLANTED;
uint8_t bomb_now = BOMB_NOT_PLANTED;
uint8_t hp = 100;
uint8_t hp_now = 100;
uint8_t ap = 0;
uint8_t ap_now = 0;
uint8_t bomb_elapsed_s = 0;

/* State variables */
bool led_update_pending = false;
bool idle_entered = true;
bool new_color_needed = true;

/******************************************************************************
 * Serial print helper function with variable argument count and type
 * ***************************************************************************/
void stprint(const char* input...)
{
  va_list args;
  va_start(args, input);
  for (const char* i = input; *i != 0; ++i)
  {
    if (*i != '%')
    {
      Serial.print(*i);
      continue;
    }
    switch (*(++i))
    {
      case '%': Serial.print('%'); break;
      case 's': Serial.print(va_arg(args, char*)); break;
      case 'd': Serial.print(va_arg(args, int), DEC); break;
      case 'b': Serial.print("b"); Serial.print(va_arg(args, int), BIN); break;
      case 'o': Serial.print("0o"); Serial.print(va_arg(args, int), OCT); break;
      case 'x': Serial.print("0x"); Serial.print(va_arg(args, int), HEX); break;
      case 'f': Serial.print(va_arg(args, double), 2); break;
    }
  }
  Serial.println();
  va_end(args);
}

/******************************************************************************
 * 404 response for incorrect web paths
 * ***************************************************************************/
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "application/json", "{\"message\":\"Not found\"}");
}

/******************************************************************************
 * Simple explosion light effect on the LED strip
 * ***************************************************************************/
void explosion_effect()
{
  /* 1st flash */
  for (uint8_t i = 0; i <= NUM_LEDS_TOTAL - 1; i++)
  {
    leds[i] = CRGB(150, 150, 0);
  }
  FastLED.show();

  delay(50);

  /* 2nd flash */
  for (uint8_t i = 0; i <= NUM_LEDS_TOTAL - 1; i++)
  {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();

  delay(30);

  /* Fade out explosion */
  for (uint8_t q = 0; q < BYTE_MAX; ++q)
  {
    for (uint8_t i = 0; i <= NUM_LEDS_TOTAL - 1; i++)
    {
      leds[i] = CRGB(BYTE_MAX-q, BYTE_MAX-q, 0);
    }
    delay(4);
    FastLED.show();
  }

  /* Small lights off period */
  delay(600);
}

/******************************************************************************
 * Random slowly sweeping color when idle.
 * TODO: limit total intensity to 255.
 * ***************************************************************************/
void idle_lights()
{
  if (new_color_needed)
  {
    uint8_t two_colors;
    uint8_t int_prim_led;
    uint8_t int_sec_led;

    /* Get new random target color */
    two_colors = random(COLOR_RG, COLOR_BR + 1);
    int_prim_led = random(0, BYTE_MAX + 1);
    int_sec_led = BYTE_MAX - int_prim_led;

    switch (two_colors)
    {
      case COLOR_RG:
        t_idle_led_r = int_prim_led;
        t_idle_led_g = int_sec_led;
        t_idle_led_b = 0;
        break;

      case COLOR_GB:
        t_idle_led_r = 0;
        t_idle_led_g = int_prim_led;
        t_idle_led_b = int_sec_led;
        break;
        
      case COLOR_BR:
        t_idle_led_r = int_sec_led;
        t_idle_led_g = 0;
        t_idle_led_b = int_prim_led;
        break;
    }
    new_color_needed = false;
    stprint("New random target color r %d, g %d, b %d. Now r %d, g %d, b %d",
            t_idle_led_r, t_idle_led_g, t_idle_led_b,
            idle_led_r, idle_led_g, idle_led_b);
  }

  /* Step current color towards the target */
  if (idle_led_r > t_idle_led_r)
  {
    idle_led_r--;
  }
  else if (idle_led_r < t_idle_led_r)
  {
    idle_led_r++;
  }

  if (idle_led_g > t_idle_led_g)
  {
    idle_led_g--;
  }
  else if (idle_led_g < t_idle_led_g)
  {
    idle_led_g++;
  }

  if (idle_led_b > t_idle_led_b)
  {
    idle_led_b--;
  }
  else if (idle_led_b < t_idle_led_b)
  {
    idle_led_b++;
  }

  /* If target reached, set flag to indicate need for a new random color */
  if (idle_led_r == t_idle_led_r &&
      idle_led_g == t_idle_led_g &&
      idle_led_b == t_idle_led_b)
  {
    new_color_needed = true;
  }

  /* Finally, set current color values to all LEDs in the strip */
  for (uint8_t i = 0; i <= NUM_LEDS_TOTAL - 1; i++)
  {
    leds[i] = CRGB(idle_led_r, idle_led_g, idle_led_b);
  }
  FastLED.show();
}

/******************************************************************************
 * Set up LED strip and web server for the CS:GO game state integration.
 * ***************************************************************************/
void setup()
{
  Serial.begin(115200);

  /* Prepare LED strip */
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS_TOTAL);

  /* Connect to WiFi */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("WiFi connection failed!");
  }
  Serial.print("WiFi connected. IP Address: ");
  Serial.println(WiFi.localIP());

  /* Add URI/path for just testing web server connectivity */
  server.on(WEB_SERVER_TEST_URI, HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(200, "application/json", "I'm here, ready and waiting.\n");
  });

  /* CS:GO game state update handler */
  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/",
  [](AsyncWebServerRequest *request, JsonVariant &json)
  {
    if (json.is<JsonObject>())
    {
      StaticJsonDocument<2048> doc;
      doc = json.as<JsonObject>();

      if (doc.containsKey("player"))
      {
        JsonObject player = doc["player"];
        JsonObject player_state = player["state"];
        hp_now = player_state["health"];
        ap_now = player_state["armor"];
      }

      if (doc.containsKey("round"))
      {
        JsonObject round = doc["round"];
        if (round.containsKey("bomb"))
        {
          bomb_state = (const char*) round["bomb"];
          if (bomb_state == "planted")
          {
            bomb_now = BOMB_PLANTED;
          }
          else if (bomb_state == "exploded")
          {
            bomb_now = BOMB_EXPLODED;
          }
          else if (bomb_state == "defused")
          {
            bomb_now = BOMB_DEFUSED;
          }
          /* Should not end up here */
          else
          {
            stprint("Warning: round.bomb has unknown value %s", bomb_state);
            bomb_now = BOMB_NOT_PLANTED;
          }
        }
      }
      /* If anything has changed, update variables to trigger LED update */
      if (bomb != bomb_now || hp != hp_now || ap != ap_now)
      {
        hp = hp_now;
        ap = ap_now;
        bomb = bomb_now;
        led_update_pending = true;
      }
    }
    else
    {
      stprint("not JsonObject!");
    }
    request->send(200);
  });

  server.addHandler(handler);
  server.onNotFound(notFound);
  server.begin();

  /* Idle lighting at startup */
  idle_lights();
}

/******************************************************************************
 * Main loop counts time and reacts to requests received by the web server.
 * ***************************************************************************/
void loop()
{
  /* Count seconds if bomb planted */
  if (last_s_ms != 0)
  {
    now_ms = millis();
    if (now_ms >= last_s_ms + 1000)
    {
      last_s_ms = now_ms;
      bomb_elapsed_s++;
      led_update_pending = true;
      stprint("Bomb elapsed %d s", bomb_elapsed_s);
    }
  }

  if (led_update_pending)
  {
    /* Is bomb planted? */
    switch (bomb)
    {
      case BOMB_NOT_PLANTED:
      case BOMB_EXPLODED:
      case BOMB_DEFUSED:
        /* Bomb has either exploded or it has been defused */
        if (last_s_ms != 0)
        {
          last_s_ms = 0;
          bomb_elapsed_s = 0;
          explosion_effect();
        }
        break;

      case BOMB_PLANTED:
        /* If counter not running, start it */
        if (last_s_ms == 0)
        {
          last_s_ms = millis();
        }
        /* Indicate bomb progress by lighting up as many leds as there are
        elapsed bomb seconds */
        for (uint8_t i = 0; i <= bomb_elapsed_s; i++)
        {
          leds[i] = CRGB(BYTE_MAX, 0, 0);
        }
        break;
    }

    /* Equations, thanks to WolframAlpha:
    Green: linear g * hp/100 * 255 where g <= 1
    Red: min(r * (100 - hp), 255) where r >= 2.55 */
    float f_green = (LED_COEFF_GREEN * ((float) hp / 100.0)) * 255.0;
    float f_red = min(LED_COEFF_RED * (100.0 - (float) hp), 255.0);
    uint8_t led_g = (uint8_t) f_green;
    uint8_t led_r = (uint8_t) f_red;
    for (uint8_t i = bomb_elapsed_s; i <= NUM_LEDS_HP - 1; i++)
    {
      leds[i] = CRGB(led_r, led_g, 0);
    }

    /* Show armor points (normalized to 255) at the end of the LED strip */
    float f_armor = ap / 100.0 * 255.0;
    uint8_t led_b = (uint8_t) f_armor;
    for (uint8_t i = NUM_LEDS_HP; i <= NUM_LEDS_TOTAL - 1; i++)
    {
      leds[i] = CRGB(0, 0, led_b);
    }
    FastLED.show();

    led_update_pending = false;

    /* Reset inactivity timer */
    inactivity_ms = millis();
    idle_entered = false;
  }

  /* Turn on inactivity lighting if needed */
  if (millis() > inactivity_ms + INACTIVITY_TIME_MS)
  {
    idle_entered = true;
  }

  /* Sweep inactivity lights */
  if (idle_entered)
  {
    if (millis() > idle_led_ms + IDLE_LIGHT_STEP_MS)
    {
      /* Reset idle color sweep timer */
      idle_led_ms = millis();
      idle_lights();
    }
  }
}
