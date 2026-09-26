#include <Arduino.h>
#include "Motors.h"
#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

const int MAX_DISTANCE_MM = 200;
const int BATTERY_MEASURE_PIN = 26;
const int LED_PIN = 23;
const int BATTERY_THRESHOLD_ADC = 2350;

Motors motors(19, 18, 16, 17);

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

void TaskBattery(void *parameter)
{
    while (1) {
        int batteryReading = analogRead(BATTERY_MEASURE_PIN);

        Serial.print("ADC value: ");
        Serial.println(batteryReading);

        if (batteryReading < BATTERY_THRESHOLD_ADC) {
            digitalWrite(LED_PIN, HIGH); // Warning: battery voltage below threshold
        } else {
            digitalWrite(LED_PIN, LOW);  // Battery voltage is acceptable
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    // Wait until the serial port opens for native USB devices
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
        TaskBattery,
        "Battery Task",
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