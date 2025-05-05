#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "pinmap.h"

typedef enum MOTOR_DIRECTION
{
    MOTOR_DIR_CW = 0,
    MOTOR_DIR_CCW = 1,
} Motor_Dir;

uint32_t pwm_set_freq(uint slice_num, uint32_t f);
void analog_signal_init();
uint16_t read_analog_signal();
void uart_signal_init();
char* read_uart_signal();
char* read_usb_serial_signal();

#endif // UTILITIES_H