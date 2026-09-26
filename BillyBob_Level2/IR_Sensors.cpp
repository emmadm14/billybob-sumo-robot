#include "IR_Sensors.h"

float filteredValues[8];

// Constructor: configure IR sensor pins
IR_Sensors::IR_Sensors(const uint8_t pins[], uint8_t count, float alpha, long refreshDelay)
    : _nbSensors(count),
      _filteredData(0),
      _alpha(alpha),
      _refreshDelay(refreshDelay),
      _rawData(0),
      _isFilteredData(false)
{
    if (_nbSensors > 8) {
        _nbSensors = 8;
    }

    if (_nbSensors == 0) {
        _nbSensors = 1;
    }

    for (uint8_t i = 0; i < _nbSensors; i++) {
        _pins[i] = pins[i];
        pinMode(_pins[i], INPUT);
        filteredValues[i] = 0.0f;
    }
}

// Get raw sensor data
uint8_t IR_Sensors::getData()
{
    return _rawData;
}

// Get filtered sensor data
uint8_t IR_Sensors::getFilteredData()
{
    return _filteredData;
}

// Enable filtering
void IR_Sensors::startFilterIR()
{
    _isFilteredData = true;
    _filteredData = 0;
}

// Disable filtering
void IR_Sensors::stopFilterIR()
{
    _isFilteredData = false;
}

// Apply EMA filtering
void IR_Sensors::filterIR(uint8_t rawData)
{
    _filteredData = 0;

    for (uint8_t i = 0; i < _nbSensors; i++) {
        uint8_t bit = (rawData >> i) & 0x01;

        filteredValues[i] =
            _alpha * bit + (1.0f - _alpha) * filteredValues[i];

        if (filteredValues[i] >= 0.5f) {
            _filteredData |= (1 << i);
        }
    }
}

// Read sensor data
void IR_Sensors::updateData()
{
    _rawData = 0;

    for (uint8_t i = 0; i < _nbSensors; i++) {
        if (digitalRead(_pins[i]) == HIGH) {
            _rawData |= (1 << i);
        }
    }

    if (_isFilteredData) {
        filterIR(_rawData);
    }
}