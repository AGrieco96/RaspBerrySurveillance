#ifndef SERVO_UTILS
	#define SERVO_UTILS

	//#include "utility_servo/soft_pwm.c"
	#include "utility_servo/soft_pwm.h"
	#include "utility_servo/gpio.h"
	//#include "utility_servo/gpio.c"
	//#include "utility_servo/event_gpio.c"
	#include "utility_servo/event_gpio.h"
	//#include "utility_servo/common.c"
	#include "utility_servo/common.h"

	#define PIN     12
	#define FREQ    50
	#define OFFSET  20

	void setupPWM(int channel);

	int setAngle(int angle);

#endif