#include <Arduino.h>
#include "IR_Sensors.h"
#include "Motors.h"
#include "Adafruit_VL53L0X.h"

#define printBinary(val, bits) \
    do { \
        for (int i = (bits) - 1; i >= 0; --i) { \
            Serial.print(((val) >> i) & 0x01); \
        } \
    } while (0)

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

const int MAX_DISTANCE_MM = 150;

const uint8_t SENSOR_COUNT = 8;
const uint8_t SENSOR_PINS[SENSOR_COUNT] = {27, 25, 32, 33, 4, 0, 34, 2};

// IR sensor zones
const uint8_t MASK_LEFT   = 0b00000111; // Bits 0, 1, 2
const uint8_t MASK_CENTER = 0b00011000; // Bits 3, 4
const uint8_t MASK_RIGHT  = 0b11100000; // Bits 5, 6, 7

// Escape manoeuvre parameters
const uint8_t SPEED_BACK = 200;
const uint8_t SPEED_TURN = 200;

const unsigned long BACK_TIME_MS  = 250;
const unsigned long SHORT_TURN_MS = 250;
const unsigned long LONG_TURN_MS  = 400;

// Ignore faulty IR sensor on bit 5
const uint8_t ERROR_MASK = ~(1 << 5);

Motors motors(19, 18, 16, 17);
IR_Sensors sensors(SENSOR_PINS, SENSOR_COUNT);

TaskHandle_t hdl_TaskFight = NULL;

void TaskFight(void *parameter)
{
    while (1) {
        VL53L0X_RangingMeasurementData_t measure;

        Serial.print("Reading a measurement... ");
        lox.rangingTest(&measure, false);

        int distance = measure.RangeMilliMeter;

        if (measure.RangeStatus != 4) {
            Serial.print("Distance (mm): ");
            Serial.println(measure.RangeMilliMeter);
        } else {
            Serial.println("Out of range");
        }

        if (distance < MAX_DISTANCE_MM) {
            motors.fight(255);
        } else {
            motors.searching(255);
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void escapeLeft(uint8_t blackMask)
{
    (void)blackMask;

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
    delay(10);

    motors.backward(Motors::LEFT, SPEED_BACK);
    motors.backward(Motors::RIGHT, SPEED_BACK);
    delay(BACK_TIME_MS);

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
    delay(10);

    motors.forward(Motors::LEFT, SPEED_TURN);
    motors.backward(Motors::RIGHT, SPEED_TURN);
    delay(SHORT_TURN_MS);

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
}

void escapeRight(uint8_t blackMask)
{
    (void)blackMask;

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
    delay(10);

    motors.backward(Motors::LEFT, SPEED_BACK);
    motors.backward(Motors::RIGHT, SPEED_BACK);
    delay(BACK_TIME_MS);

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
    delay(10);

    motors.backward(Motors::LEFT, SPEED_TURN);
    motors.forward(Motors::RIGHT, SPEED_TURN);
    delay(SHORT_TURN_MS);

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
}

void escapeCenter(uint8_t blackMask)
{
    (void)blackMask;

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
    delay(10);

    motors.backward(Motors::LEFT, SPEED_BACK);
    motors.backward(Motors::RIGHT, SPEED_BACK);
    delay(BACK_TIME_MS);

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
    delay(10);

    motors.forward(Motors::LEFT, SPEED_TURN);
    motors.backward(Motors::RIGHT, SPEED_TURN);
    delay(SHORT_TURN_MS);

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
}

void escapeUnknown(uint8_t blackMask)
{
    (void)blackMask;

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
    delay(10);

    motors.backward(Motors::LEFT, SPEED_BACK);
    motors.backward(Motors::RIGHT, SPEED_BACK);
    delay(BACK_TIME_MS + 100);

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
    delay(10);

    motors.forward(Motors::LEFT, SPEED_TURN);
    motors.backward(Motors::RIGHT, SPEED_TURN);
    delay(LONG_TURN_MS);

    motors.stop(Motors::LEFT);
    motors.stop(Motors::RIGHT);
}

void handleEscape(uint8_t blackMask)
{
    if (blackMask == 0) {
        return;
    }

    uint8_t activeSensors = 0;

    for (int i = 0; i < SENSOR_COUNT; ++i) {
        if ((blackMask >> i) & 0x01) {
            activeSensors++;
        }
    }

    if (activeSensors >= 6) {
        escapeUnknown(blackMask);
        return;
    }

    uint8_t blackLeft   = blackMask & MASK_LEFT;
    uint8_t blackCenter = blackMask & MASK_CENTER;
    uint8_t blackRight  = blackMask & MASK_RIGHT;

    if (blackLeft && !blackRight) {
        escapeLeft(blackMask);
    } else if (blackRight && !blackLeft) {
        escapeRight(blackMask);
    } else if (blackCenter) {
        escapeCenter(blackMask);
    } else {
        escapeUnknown(blackMask);
    }
}

void TaskBorder(void *parameter)
{
    while (1) {
        sensors.updateData();

        uint8_t rawData = sensors.getData();
        uint8_t filteredData = sensors.getFilteredData();
        uint8_t correctedData = filteredData & ERROR_MASK;

        Serial.print("Raw value = ");
        printBinary(rawData, SENSOR_COUNT);

        Serial.print("    Filtered value = ");
        printBinary(filteredData, SENSOR_COUNT);

        Serial.print("    Corrected value = ");
        printBinary(correctedData, SENSOR_COUNT);

        Serial.println();

        uint8_t blackMask = correctedData;

        if (blackMask != 0) {
            vTaskSuspend(hdl_TaskFight);

            handleEscape(blackMask);

            vTaskResume(hdl_TaskFight);
        } else {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}

void setup()
{
    Serial.begin(115200);

    while (!Serial) {
        delay(1);
    }

    Serial.println("BillyBob - VL53L0X initialization");

    if (!lox.begin()) {
        Serial.println(F("Failed to boot VL53L0X"));

        while (1) {
        }
    }

    Serial.println(F("VL53L0X ready"));

    Serial.println("IR sensors - EMA filtering enabled");
    sensors.startFilterIR();

    xTaskCreatePinnedToCore(
        TaskFight,
        "Fight Task",
        2048,
        NULL,
        1,
        &hdl_TaskFight,
        1
    );

    xTaskCreatePinnedToCore(
        TaskBorder,
        "Border Task",
        2048,
        NULL,
        2,
        NULL,
        1
    );
}

void loop()
{
}