#include "esp32-hal-gpio.h"
#include <stdint.h>
#include "Arduino.h"
#include "Motors.h"
#include "esp32-hal-ledc.h"
#include "esp32-hal.h"

Motors::Motors(uint8_t motor_left_a, uint8_t motor_left_b, uint8_t motor_left_enable, uint8_t motor_right_a, uint8_t motor_right_b, uint8_t motor_right_enable, int frequency, uint8_t resolution) {
  MOTOR_LEFT_A = motor_left_a;
  MOTOR_LEFT_B = motor_left_b;
  MOTOR_LEFT_EN = motor_left_enable;

  MOTOR_RIGHT_A = motor_right_a;
  MOTOR_RIGHT_B = motor_right_b;
  MOTOR_RIGHT_EN = motor_right_enable;

  FREQUENCY = frequency;
  RESOLUTION = resolution;

  // Initializa left motor pins
  pinMode(MOTOR_LEFT_A, OUTPUT);
  pinMode(MOTOR_LEFT_B, OUTPUT);
  pinMode(MOTOR_LEFT_EN, OUTPUT);
  ledcAttach(MOTOR_LEFT_EN, FREQUENCY, RESOLUTION);

  // Initializa right motor pins
  pinMode(MOTOR_RIGHT_A, OUTPUT);
  pinMode(MOTOR_RIGHT_B, OUTPUT);
  pinMode(MOTOR_RIGHT_EN, OUTPUT);
  ledcAttach(MOTOR_RIGHT_EN, FREQUENCY, RESOLUTION);

  // Set motors on stop
  ledcWrite(MOTOR_LEFT_EN, 0);
  ledcWrite(MOTOR_RIGHT_EN, 0);

  digitalWrite(MOTOR_LEFT_A, LOW);
  digitalWrite(MOTOR_LEFT_B, LOW);

  digitalWrite(MOTOR_RIGHT_A, LOW);
  digitalWrite(MOTOR_RIGHT_B, LOW);
}

void Motors::driveMotors(int left_speed, int right_speed) {
  driveLeftMotor(left_speed);
  driveRightMotor(right_speed);
}

void Motors::driveLeftMotor(int speed) {
  if (speed >= 0) {
    digitalWrite(MOTOR_LEFT_A, HIGH);
    digitalWrite(MOTOR_LEFT_B, LOW);
  } else {
    digitalWrite(MOTOR_LEFT_A, LOW);
    digitalWrite(MOTOR_LEFT_B, HIGH);
  }

  ledcWrite(MOTOR_LEFT_EN, abs(speed));
}

void Motors::driveRightMotor(int speed) {
  if (speed >= 0) {
    digitalWrite(MOTOR_RIGHT_A, HIGH);
    digitalWrite(MOTOR_RIGHT_B, LOW);
  } else {
    digitalWrite(MOTOR_RIGHT_A, LOW);
    digitalWrite(MOTOR_RIGHT_B, HIGH);
  }

  ledcWrite(MOTOR_RIGHT_EN, abs(speed));
}
