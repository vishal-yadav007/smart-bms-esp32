```cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <mcp_can.h>
#include <DHT.h>

// =====================================================
// WIFI CONFIGURATION
// Replace these locally before uploading to ESP32
// DO NOT COMMIT REAL CREDENTIALS TO GITHUB
// =====================================================

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// =====================================================
// FIREBASE CONFIGURATION
// Replace with your Firebase Realtime Database URL
// DO NOT COMMIT private credentials or secrets
// =====================================================

const char* FIREBASE_URL =
    "https://YOUR_FIREBASE_PROJECT-default-rtdb.REGION.firebasedatabase.app/BMS.json";

// =====================================================
// MCP2515 CAN
// =====================================================

#define CAN_CS_PIN 5
#define CAN_SCK    18
#define CAN_MISO   19
#define CAN_MOSI   23

#define CAN_ID 0x100

MCP_CAN CAN0(CAN_CS_PIN);

// =====================================================
// DHT22
// =====================================================

#define DHT_PIN 27
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

// =====================================================
// RELAY / FAN
// =====================================================

#define RELAY_PIN 26

// Active LOW relay
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// =====================================================
// FAN TEMPERATURE LIMIT
// =====================================================

// Fan ON at 32°C or above
// Fan OFF below 32°C

#define FAN_TEMPERATURE 32.0

// =====================================================
// LOW VOLTAGE LIMIT
// =====================================================

#define LOW_VOLTAGE_LIMIT 5.0

// =====================================================
// VARIABLES
// =====================================================

float batteryVoltage = 0.0;
float batteryCurrent = 0.0;
float batteryPower = 0.0;

float temperature = 0.0;
float humidity = 0.0;

float soc = 0.0;

bool fanStatus = false;
bool dhtValid = false;

// =====================================================
// TIMERS
// =====================================================

unsigned long lastDHTRead = 0;
unsigned long lastFirebaseUpload = 0;
unsigned long lastWiFiCheck = 0;

const unsigned long DHT_INTERVAL = 2000;
const unsigned long FIREBASE_INTERVAL = 2000;
const unsigned long WIFI_CHECK_INTERVAL = 10000;

// =====================================================
// WIFI CONNECTION
// =====================================================

void connectWiFi()
{
    Serial.println();
    Serial.println("Connecting to Wi-Fi...");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;

    while (WiFi.status() != WL_CONNECTED && attempts < 30)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Wi-Fi Connected!");

        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("Wi-Fi connection FAILED");
    }
}

// =====================================================
// WIFI CHECK
// =====================================================

void checkWiFi()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println();
        Serial.println("Wi-Fi disconnected!");
        Serial.println("Reconnecting...");

        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
}

// =====================================================
// RECEIVE CAN DATA
// =====================================================

void receiveCANData()
{
    if (CAN0.checkReceive() == CAN_MSGAVAIL)
    {
        unsigned long rxId;

        byte len = 0;
        byte rxBuf[8];

        CAN0.readMsgBuf(&rxId, &len, rxBuf);

        if (rxId == CAN_ID)
        {
            if (len >= 4)
            {
                uint16_t voltageData =
                    ((uint16_t)rxBuf[0] << 8) |
                    rxBuf[1];

                uint16_t currentData =
                    ((uint16_t)rxBuf[2] << 8) |
                    rxBuf[3];

                batteryVoltage =
                    voltageData / 100.0;

                batteryCurrent =
                    currentData / 100.0;

                batteryPower =
                    batteryVoltage * batteryCurrent;

                Serial.println();
                Serial.println("================================");
                Serial.println("       CAN DATA RECEIVED");
                Serial.println("================================");

                Serial.print("Voltage : ");
                Serial.print(batteryVoltage, 2);
                Serial.println(" V");

                Serial.print("Current : ");
                Serial.print(batteryCurrent, 2);
                Serial.println(" A");

                Serial.print("Power   : ");
                Serial.print(batteryPower, 2);
                Serial.println(" W");

                if (batteryVoltage <= LOW_VOLTAGE_LIMIT)
                {
                    Serial.println();
                    Serial.println("!!! CRITICAL LOW VOLTAGE !!!");
                    Serial.println("Voltage <= 5.0 V");
                }
                else
                {
                    Serial.println("Voltage Status: NORMAL");
                }

                Serial.println("================================");
            }
        }
    }
}

// =====================================================
// READ DHT22
// =====================================================

void readDHT22()
{
    float newTemperature = dht.readTemperature();
    float newHumidity = dht.readHumidity();

    if (isnan(newTemperature) || isnan(newHumidity))
    {
        Serial.println();
        Serial.println("DHT22 SENSOR ERROR!");

        Serial.println("Temperature : SENSOR ERROR");
        Serial.println("Humidity    : SENSOR ERROR");

        dhtValid = false;

        // Safety: fan
