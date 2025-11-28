#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define LED_PIN 48
#define LED_COUNT 1
#define CHANNEL 1

const uint8_t RECEIVERMAC[] = {0x20, 0x6E, 0xF1, 0xB0, 0xF8, 0x34};

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

const int btnPins[6] = {4, 5, 6, 8, 9, 10};
const uint32_t colors[6] = {
	strip.Color(255, 0, 0),	  // Red
	strip.Color(0, 255, 0),	  // Green
	strip.Color(0, 0, 255),	  // Blue
	strip.Color(255, 255, 0), // Yellow
	strip.Color(255, 0, 255), // Magenta
	strip.Color(0, 255, 255)  // Cyan
};

unsigned long lastHeartbeat = 0;
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
	Serial.print("Last Packet Send Status: ");
	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void SetupEspNowPeer()
{
	esp_now_peer_info_t peerInfo = {};
	memcpy(peerInfo.peer_addr, RECEIVERMAC, 6);
	peerInfo.channel = CHANNEL;
	peerInfo.encrypt = false;

	esp_now_del_peer(peerInfo.peer_addr);
	if (esp_now_add_peer(&peerInfo) != ESP_OK)
	{
		Serial.println("Failed to add peer");
		return;
	}
	else
	{
		Serial.println("Peer added successfully");
	}
}

void setup()
{
	Serial.begin(115200);
	delay(1000);

	strip.begin();
	strip.setBrightness(50); // Set brightness to 50%
	strip.show();

	for (int i = 0; i < 6; i++)
	{
		pinMode(btnPins[i], INPUT_PULLUP);
	}

	WiFi.mode(WIFI_STA);
	esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
	WiFi.disconnect();

	if (esp_now_init() != ESP_OK)
	{
		log("Error initializing ESP-NOW");
		return;
	}

	esp_now_register_send_cb(OnDataSent);
	SetupEspNowPeer();

	Serial.println("Peer MAC Address: ");
	for (int i = 0; i < 6; i++)
	{
		Serial.print(RECEIVERMAC[i], HEX);
		if (i < 5)
			Serial.print(":");
	}
	Serial.println();

	log("Initialization complete.");
}

void loop()
{
	if (!peerConnected)
	{
		if (millis() - lastBlinkTime >= 500)
		{
			blinkState = !blinkState;
			strip.setPixelColor(0, blinkState ? strip.Color(255, 0, 0) : strip.Color(0, 0, 0));
			strip.show();
			lastBlinkTime = millis();
		}
	}

	bool anyButtonPressed = false;

	for (int i = 0; i < 6; i++)
	{
		if (digitalRead(btnPins[i]) == LOW)
		{
			anyButtonPressed = true;

			if (peerConnected)
			{
				strip.setPixelColor(0, colors[i]);
				strip.show();
			}

			log("Button IO" + String(btnPins[i]) + " pressed. Sending color index: " + String(i));

			uint8_t buttonData = i;
			esp_now_send(RECEIVERMAC, &buttonData, sizeof(buttonData));
			delay(500); // Debounce delay
			break;		// Only process one button at a time
		}
	}

	// Send "off" command when no buttons are pressed
	if (peerConnected && !anyButtonPressed)
	{
		static bool wasPressed = false;

		if (wasPressed)
		{
			strip.setPixelColor(0, strip.Color(0, 0, 0));
			strip.show();

			uint8_t offData = 6; // Send index 6 to turn off
			esp_now_send(RECEIVERMAC, &offData, sizeof(offData));
			log("All buttons released. Sending OFF command");

			wasPressed = false;
		}
	}
	else if (anyButtonPressed)
	{
		static bool wasPressed = false;
		wasPressed = true;
	}
}