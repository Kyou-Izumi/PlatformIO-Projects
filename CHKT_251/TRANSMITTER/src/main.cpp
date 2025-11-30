/* Transmitter (TX)
   - Reads 6 buttons (active-low), sends PRESS/RELEASE messages via ESP-NOW
   - Local NeoPixel indicator shows the same color when pressing
   - Set RECEIVER_MAC to the receiver's MAC (6 bytes)
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// -------------------------------------------
// CONFIG
// -------------------------------------------

// LED strip parameters
#define LED_PIN 48
#define LED_COUNT 1
#define LED_BRIGHTNESS 30

// WiFi channel
#define CHANNEL 1

// -------------------------------------------
// GLOBALS
// -------------------------------------------
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// MAC address of the receiver
const uint8_t RECEIVERMAC[] = {0x20, 0x6E, 0xF1, 0xB0, 0xF8, 0x34};

// Packet definitions
struct CommandPacket
{
	uint8_t servo;
	int8_t direction; // +1 = clockwise, -1 = counterclockwise, 0 = stop
};

// Button mapping
struct BtnMap
{
	uint8_t pin;
	uint8_t servo;
	int8_t direction;
};

BtnMap mapList[6] = {
	{8, 0, -1},	 // Button on pin 8 controls servo 0 counterclockwise
	{6, 0, +1},	 // Button on pin 6 controls servo 0 clockwise
	{9, 1, -1},	 // Button on pin 9 controls servo 1 counterclockwise
	{5, 1, +1},	 // Button on pin 5 controls servo 1 clockwise
	{10, 2, -1}, // Button on pin 10 controls servo 2 counterclockwise
	{4, 2, +1}	 // Button on pin 4 controls servo 2 clockwise
};

// LED mapping
struct LedMap
{
	uint8_t pin;
	uint32_t color;
};

LedMap ledList[6] = {
	{8, strip.Color(255, 0, 0)},	// LED for button 0
	{6, strip.Color(0, 255, 0)},	// LED for button 1
	{9, strip.Color(0, 0, 255)},	// LED for button 2
	{5, strip.Color(255, 255, 0)},	// LED for button 3
	{10, strip.Color(255, 0, 255)}, // LED for button 4
	{4, strip.Color(0, 255, 255)}	// LED for button 5
};

bool pressedState[6] = {false, false, false, false, false, false};
unsigned long lastDebounce[6] = {0};

const unsigned long DEBOUNCE_MS = 50UL;
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL = 5000UL;


// -------------------------------------------
// SEND CALLBACK
// -------------------------------------------
void OnSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
	Serial.print("Last Packet Send Status: ");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void sendPacket(uint8_t servo, int8_t direction)
{
	CommandPacket p;
	p.servo = servo;
	p.direction = direction;

	esp_now_send(RECEIVERMAC, (uint8_t *)&p, sizeof(p));
}

// -------------------------------------------
// SETUP
// -------------------------------------------
void setup()
{
	Serial.begin(115200);
	delay(1000);

	strip.begin();
	strip.setBrightness(LED_BRIGHTNESS);
	strip.show();

	for (int i = 0; i < 6; i++)
	{
		pinMode(mapList[i].pin, INPUT_PULLUP);
		pressedState[i] = digitalRead(mapList[i].pin) == LOW;
		lastDebounce[i] = millis();
	}

	WiFi.mode(WIFI_STA);
	esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);

	WiFi.setSleep(true);
	WiFi.setTxPower(WIFI_POWER_8_5dBm); // 8.5 dBm = 7 mW
	WiFi.disconnect();

	if (esp_now_init() != ESP_OK)
	{
		Serial.println("Error initializing ESP-NOW");
		return;
	}

	esp_now_register_send_cb(OnSent);

	esp_now_peer_info_t peer{};
	memcpy(peer.peer_addr, RECEIVERMAC, 6);
	peer.channel = CHANNEL;
	peer.encrypt = false;

	esp_now_add_peer(&peer);

	Serial.println("Initialization complete.");
}

// -------------------------------------------
// LOOP
// -------------------------------------------
void loop()
{
	unsigned long now = millis();

	for (int i = 0; i < 6; i++)
	{
		bool pressed = digitalRead(mapList[i].pin) == LOW;
		if (pressed != pressedState[i] && now - lastDebounce[i] > DEBOUNCE_MS)
		{
			pressedState[i] = pressed;
			lastDebounce[i] = now;

			// Update LED
			strip.setPixelColor(0, pressed ? ledList[i].color : 0);
			strip.show();

			// Send command packet
			CommandPacket p;
			p.servo = mapList[i].servo;
			p.direction = pressed ? mapList[i].direction : 0;

			esp_now_send(RECEIVERMAC, (uint8_t *)&p, sizeof(p));

			Serial.printf("Btn %d -> servo %d dir %d\n", i, p.servo, p.direction);
		}
	}

	// Heartbeat every interval
	if (now - lastHeartbeat > HEARTBEAT_INTERVAL)
	{
		CommandPacket hb = {255, 0}; // Heartbeat packet
		esp_now_send(RECEIVERMAC, (uint8_t *)&hb, sizeof(hb));
		lastHeartbeat = now;
	}

	vTaskDelay(1);
}