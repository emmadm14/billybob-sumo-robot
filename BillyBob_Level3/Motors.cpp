#include "Motors.h"

Motors* _instance = nullptr;

volatile long ticksM1 = 0;
volatile long ticksM2 = 0;

Motors::Motors(uint8_t gpioIn1, uint8_t gpioIn2, uint8_t gpioIn3, uint8_t gpioIn4)
    : _gpioIn1(gpioIn1),
      _gpioIn2(gpioIn2),
      _gpioIn3(gpioIn3),
      _gpioIn4(gpioIn4)
{
    pinMode(_gpioIn1, OUTPUT);
    pinMode(_gpioIn2, OUTPUT);
    pinMode(_gpioIn3, OUTPUT);
    pinMode(_gpioIn4, OUTPUT);

    _motorLeftMove = 0;
    _motorRightMove = 0;

    ticksM1 = 0;
    ticksM2 = 0;
}

void encoderM1ISR()
{
    if (_instance) {
        ticksM1++;
    }
}

void encoderM2ISR()
{
    if (_instance) {
        ticksM2++;
    }
}

void Motors::attachEncoders(uint8_t encoderM1, uint8_t encoderM2)
{
    pinMode(encoderM1, INPUT_PULLUP);
    pinMode(encoderM2, INPUT_PULLUP);

    _instance = this;

    ticksM1 = 0;
    ticksM2 = 0;

    attachInterrupt(digitalPinToInterrupt(encoderM1), encoderM1ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(encoderM2), encoderM2ISR, RISING);
}

long Motors::getTicks(uint8_t motor)
{
    if (motor == LEFT) {
        return ticksM1;
    }

    return ticksM2;
}

void Motors::forward(uint8_t motor, uint8_t speed)
{
    if (motor == LEFT) {
        digitalWrite(_gpioIn1, speed);
        digitalWrite(_gpioIn2, LOW);
        _motorLeftMove = 1;
    } else {
        digitalWrite(_gpioIn3, speed);
        digitalWrite(_gpioIn4, LOW);
        _motorRightMove = 1;
    }
}

void Motors::backward(uint8_t motor, uint8_t speed)
{
    if (motor == LEFT) {
        digitalWrite(_gpioIn1, LOW);
        digitalWrite(_gpioIn2, speed);
        _motorLeftMove = 2;
    } else {
        digitalWrite(_gpioIn3, LOW);
        digitalWrite(_gpioIn4, speed);
        _motorRightMove = 2;
    }
}

void Motors::stop(uint8_t motor)
{
    if (motor == LEFT) {
        analogWrite(_gpioIn1, 0);
        analogWrite(_gpioIn2, 0);
        _motorLeftMove = 0;
    } else {
        analogWrite(_gpioIn3, 0);
        analogWrite(_gpioIn4, 0);
        _motorRightMove = 0;
    }
}

void Motors::fight(uint8_t speed)
{
    forward(LEFT, speed);
    forward(RIGHT, speed);
}

void Motors::searching(uint8_t speed)
{
    backward(LEFT, speed);
    forward(RIGHT, speed);
}

bool Motors::isMotorStopped(uint8_t motor)
{
    if (motor == LEFT) {
        return _motorLeftMove == 0;
    }

    return _motorRightMove == 0;
}

bool Motors::isMotorForward(uint8_t motor)
{
    if (motor == LEFT) {
        return _motorLeftMove == 1;
    }

    return _motorRightMove == 1;
}

bool Motors::isMotorBackward(uint8_t motor)
{
    if (motor == LEFT) {
        return _motorLeftMove == 2;
    }

    return _motorRightMove == 2;
}