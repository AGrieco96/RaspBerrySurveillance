#!/usr/bin/env python

import sys
import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BOARD)
GPIO.setup(32, GPIO.OUT)
pwm=GPIO.PWM(32, 50)
pwm.start(0)

def SetAngle(angle):
	duty = angle / 18 + 2
	GPIO.output(32, True)
	pwm.ChangeDutyCycle(duty)
	sleep(1)
	GPIO.output(32, False)
	pwm.ChangeDutyCycle(0)

angle = int(sys.argv[1])+90
SetAngle(angle)
pwm.stop()
GPIO.cleanup()
