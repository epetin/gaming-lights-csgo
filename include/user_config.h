/******************************************************************************
 * GAMING-LIGHTS-CSGO
 * USER CONTROLLED VARIABLES
 * ***************************************************************************/
/* Web server port */
#define ESP_WEB_SERVER_PORT 80
#define WEB_SERVER_TEST_URI "/test"

/* WiFi information */
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

/* Inactivity threshold and idle state color transition step duration */
#define INACTIVITY_TIME_MS 120000
#define IDLE_LIGHT_STEP_MS 2000

/* LED strip details */
#define LED_PIN 15
#define NUM_LEDS_TOTAL 60
#define NUM_LEDS_HP 40
#define NUM_LEDS_AP 20

/* CS:GO health point color curve coefficients */
#define LED_COEFF_RED 4.0  // must be >= 2.55
#define LED_COEFF_GREEN 0.8  // must be <= 1