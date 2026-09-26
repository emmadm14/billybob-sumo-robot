/**
 * Date: 13/11/2025
 * Version: 1
 * Author: E. Da Mota
 */

#ifndef MOTORS_CONTROL_H
#define MOTORS_CONTROL_H

#include <Arduino.h>
#include <stdint.h>

class Motors {
public:
    static const uint8_t LEFT = 0;
    static const uint8_t RIGHT = 1;

    Motors(uint8_t gpioIn1, uint8_t gpioIn2, uint8_t gpioIn3, uint8_t gpioIn4);

    void attachEncoders(uint8_t encoderM1, uint8_t encoderM2);

    long getTicks(uint8_t motor);

    void forward(uint8_t motor, uint8_t speed);
    void backward(uint8_t motor, uint8_t speed);
    void stop(uint8_t motor);

    void fight(uint8_t speed);
    void searching(uint8_t speed);

    bool isMotorStopped(uint8_t motor);
    bool isMotorForward(uint8_t motor);
    bool isMotorBackward(uint8_t motor);

private:
    uint8_t _motorLeftMove = 0;
    uint8_t _motorRightMove = 0;

    uint8_t _gpioIn1;
    uint8_t _gpioIn2;
    uint8_t _gpioIn3;
    uint8_t _gpioIn4;
};

#endif // MOTORS_CONTROL_H