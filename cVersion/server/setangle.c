#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "setangle.h"

unsigned int gpio;

void setupPWM(int channel){
    int direction = OUTPUT;
    int initial = LOW;
    int pud = PUD_OFF + OFFSET;
    int func;

    int ret = setup();
    fprintf(stderr, "ret : %d\n", ret);
    module_setup = 1;

    get_gpio_number(channel , &gpio);
    fprintf(stderr, "Gpio Recuperato\n" );

    func = gpio_function(gpio);
    fprintf(stderr, "func fatto\n");
    output_gpio(gpio, initial);
    fprintf(stderr, "output fatto\n");
    setup_gpio(gpio, direction, pud);
    fprintf(stderr, "setup fatto\n");
}

int setAngle(int angle){
    setupPWM(PIN);
    //PWM
    pwm_set_frequency(gpio, FREQ);
    //start
    pwm_set_duty_cycle(gpio , 0);
    pwm_start(gpio);
    /**     setangle     **/
    //get duty
    float duty = ((float)angle)/18 + 2;
    
    output_gpio(gpio , HIGH);
    pwm_set_duty_cycle(gpio, duty);

    sleep(1);

    output_gpio(gpio , LOW);
    pwm_set_duty_cycle(gpio , 0);
    //stop
    pwm_stop(gpio);
    //cleanup
    // clean up any /sys/class exports
    event_cleanup(gpio);
    // set everything back to input
    setup_gpio(gpio, INPUT, PUD_OFF);
}
