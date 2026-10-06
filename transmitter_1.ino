#include <SPI.h>
#include <mcp_can.h>

// =====================================================
// PIN DEFINITIONS
// =====================================================

#define VOLTAGE_PIN 34
#define CURRENT_PIN 35
#define CAN_CS_PIN 5

// CAN SPI
#define CAN_SCK  18
#define CAN_MISO 19
#define CAN_MOSI 23

// =====================================================
// CAN SETTINGS
// =====================================================

#define CAN_ID 0x100
#define CAN_BAUD CAN_500KBPS
#define CAN_CRYSTAL MCP_8MHZ

MCP_CAN CAN0(CAN_CS_PIN);

// =====================================================
// VOLTAGE SENSOR
// =====================================================

float voltageFactor = 5.0;

// Critical alarm only
#define LOW_VOLTAGE_LIMIT 5.0

// =====================================================
// ACS712 CURRENT SENSOR
// =====================================================

float currentSensitivity = 0.100;
float currentZeroVoltage = 1.65;

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("       SMART BMS TRANSMITTER");
    Serial.println("================================");

    // ADC
    analogReadResolution(12);

    analogSetPinAttenuation(
        VOLTAGE_PIN,
        ADC_11db
    );

    analogSetPinAttenuation(
        CURRENT_PIN,
        ADC_11db
    );

    // SPI
    SPI.begin(
        CAN_SCK,
        CAN_MISO,
        CAN_MOSI,
        CAN_CS_PIN
    );

    Serial.println("Initializing CAN...");

    // CAN initialization
    if (
        CAN0.begin(
            MCP_ANY,
            CAN_BAUD,
            CAN_CRYSTAL
        ) == CAN_OK
    )
    {
        Serial.println(
            "CAN Initialization SUCCESS"
        );
    }
    else
    {
        Serial.println(
            "CAN Initialization FAILED"
        );

        while (1)
        {
            delay(1000);
        }
    }

    CAN0.setMode(MCP_NORMAL);

    Serial.println("CAN Mode: NORMAL");
    Serial.println("CAN ID: 0x100");
    Serial.println("CAN Speed: 500 kbps");
    Serial.println("MCP2515 Crystal: 8 MHz");

    Serial.println();
    Serial.println("Transmitter Ready");
    Serial.println("================================");
}

// =====================================================
// READ BATTERY VOLTAGE
// =====================================================

float readVoltage()
{
    long total = 0;

    for (int i = 0; i < 20; i++)
    {
        total += analogRead(VOLTAGE_PIN);

        delay(2);
    }

    float raw = total / 20.0;

    float sensorVoltage =
        (raw / 4095.0) * 3.3;

    float batteryVoltage =
        sensorVoltage * voltageFactor;

    return batteryVoltage;
}

// =====================================================
// READ CURRENT
// =====================================================

float readCurrent()
{
    long total = 0;

    for (int i = 0; i < 20; i++)
    {
        total += analogRead(CURRENT_PIN);

        delay(2);
    }

    float raw = total / 20.0;

    float sensorVoltage =
        (raw / 4095.0) * 3.3;

    float current =
        (sensorVoltage -
         currentZeroVoltage)
        / currentSensitivity;

    // Remove negative sign
    current = abs(current);

    return current;
}

// =====================================================
// SEND CAN DATA
// =====================================================

void sendCANData(
    float voltage,
    float current
)
{
    // Convert to integer
    // Example:
    // 12.35 V -> 1235
    // 2.45 A  -> 245

    uint16_t voltageData =
        (uint16_t)(voltage * 100);

    uint16_t currentData =
        (uint16_t)(current * 100);

    byte data[4];

    data[0] = highByte(voltageData);
    data[1] = lowByte(voltageData);

    data[2] = highByte(currentData);
    data[3] = lowByte(currentData);

    byte result =
        CAN0.sendMsgBuf(
            CAN_ID,
            0,
            4,
            data
        );

    if (result == CAN_OK)
    {
        Serial.println(
            "CAN: Message Sent"
        );
    }
    else
    {
        Serial.println(
            "CAN: Send ERROR"
        );
    }
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
    float batteryVoltage =
        readVoltage();

    float batteryCurrent =
        readCurrent();

    float batteryPower =
        batteryVoltage *
        batteryCurrent;

    Serial.println();
    Serial.println("----------------------------");

    Serial.print("Voltage : ");
    Serial.print(
        batteryVoltage,
        2
    );
    Serial.println(" V");

    Serial.print("Current : ");
    Serial.print(
        batteryCurrent,
        2
    );
    Serial.println(" A");

    Serial.print("Power   : ");
    Serial.print(
        batteryPower,
        2
    );
    Serial.println(" W");

    // Critical voltage warning
    if (
        batteryVoltage <=
        LOW_VOLTAGE_LIMIT
    )
    {
        Serial.println();
        Serial.println(
            "!!! CRITICAL WARNING !!!"
        );

        Serial.println(
            "Battery Voltage <= 5.0 V"
        );

        Serial.println(
            "Check battery and wiring!"
        );
    }
    else
    {
        Serial.println(
            "Voltage Status: NORMAL"
        );
    }

    // Send data
    sendCANData(
        batteryVoltage,
        batteryCurrent
    );

    delay(1000);
}