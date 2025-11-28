#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <esp_wifi.h>

#define LED_PIN 48
#define LED_COUNT 1
#define CHANNEL 1

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

const uint32_t colors[6] = {
    strip.Color(255, 0, 0),   // Red
    strip.Color(0, 255, 0),   // Green
    strip.Color(0, 0, 255),   // Blue
    strip.Color(255, 255, 0), // Yellow
    strip.Color(255, 0, 255), // Magenta
    strip.Color(0, 255, 255)  // Cyan
};

unsigned long lastPacketTime = 0;
const unsigned long TIMEOUT_MS = 1500;
bool lostConnection = false;

void onDataReceived(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    lastPacketTime = millis();
    lostConnection = false;

    if (data[0] == 0xFE)
    {
        // heartbeat, do nothing except update lastPacketTime
        return;
    }

    if (data_len != sizeof(uint8_t))
    {
        Serial.println("Invalid data length");
        return;
    }

    uint8_t colorIndex = data[0];
    if (colorIndex < 6)
    {
        strip.setPixelColor(0, colors[colorIndex]);
        strip.show();
        Serial.print("Changed color to index: ");
        Serial.println(colorIndex);
    }
    else if (colorIndex == 0xFF)
    {
        strip.setPixelColor(0, 0); // Turn off
        strip.show();
        Serial.println("Turned off the LED");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    strip.begin();
    strip.setBrightness(50);
    strip.show();

    WiFi.mode(WIFI_STA);
    esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
    WiFi.disconnect(); // Ensure we're not connected to any AP

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(onDataReceived);

    Serial.println("Initial MAC Address: " + WiFi.macAddress());
    Serial.println("Initialization complete.");
}

void loop()
{
    unsigned long now = millis();

    if (!lostConnection && (now - lastPacketTime > TIMEOUT_MS))
    {
        lostConnection = true;
        Serial.println("Connection lost.");
    }

    if (lostConnection)
    {
        static unsigned long lastBlinkTime = 0;
        static bool ledState = false;

        if (now - lastBlinkTime >= 500)
        {
            lastBlinkTime = now;
            ledState = !ledState;

            strip.setPixelColor(0, ledState ? strip.Color(255, 0, 0) : 0); // Blink red
            strip.show();
        }
    }

    delay(10);
}