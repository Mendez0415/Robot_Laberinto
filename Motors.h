#include <stdint.h>
#include "esp32-hal.h"
#ifndef Motors_h
#define Motors_h

class Motors {
  // function to initialize motors & PWM settings
public:
  Motors(uint8_t motor_left_a, uint8_t motor_left_b, uint8_t motor_left_enable, uint8_t motor_right_a, uint8_t motor_right_b, uint8_t motor_right_enable, int frequency, uint8_t resolution);
  void driveMotors(int left_speed, int right_speed);

private:
  uint8_t MOTOR_LEFT_A, MOTOR_LEFT_B, MOTOR_LEFT_EN, MOTOR_RIGHT_A, MOTOR_RIGHT_B, MOTOR_RIGHT_EN, FREQUENCY, RESOLUTION;
  void driveLeftMotor(int speed);
  void driveRightMotor(int speed);
};

#endif