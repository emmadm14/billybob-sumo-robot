/**
 * Date: 25/11/2025
 * Version: 1
 * Author: E. Da Mota
 */

#ifndef IR_SENSORS_H
#define IR_SENSORS_H

#include <Arduino.h>
#include <stdint.h>

class IR_Sensors {
public:
    IR_Sensors(const uint8_t pins[], uint8_t count, float alpha = 0.2f, long refreshDelay = 20L);

    uint8_t getData();          // Read raw sensor bits
    uint8_t getFilteredData();  // Read filtered sensor bits

    void stopFilterIR();        // Currently unused
    void startFilterIR();       // Currently unused
    void updateData();          // Called automatically by timer interrupt

private:
    void filterIR(uint8_t raw); // Apply EMA filtering

    uint8_t _pins[8];           // GPIO pins used to read the IR sensors
    bool _isFilteredData;

    uint8_t _nbSensors;         // Number of IR sensors

    volatile uint8_t _rawData;
    volatile uint8_t _filteredData;

    float _alpha;
    long _refreshDelay;         // Update period in ms
};

#endif // IR_SENSORS_H