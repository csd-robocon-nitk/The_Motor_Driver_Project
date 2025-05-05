#ifndef BDC_DRIVE_H
#define BDC_DRIVE_H

#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/pwm.h"
#include "pinmap.h"
#include "config.h"
#include "utilities.h"

void bdc_init();
void update_tick(uint gpio, uint32_t event_mask);
bool pid_update(__unused struct repeating_timer *t);
void bdc_move();
void set_bdc_pwm_from_analog(uint16_t val);
void set_bdc_pwm_from_uart(char *val);

#endif // BDC_DRIVE_H