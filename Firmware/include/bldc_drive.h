#ifndef BLDC_DRIVE_H
#define BLDC_DRIVE_H

#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/pwm.h"
#include "pinmap.h"
#include "config.h"
#include "utilities.h"

extern volatile bool is_stall;

void bldc_init();
void bldc_move_cw();
void bldc_move_ccw();
int64_t stall_protection(alarm_id_t id, __unused void *user_data);
void update_step(uint gpio, uint32_t event_mask);
void set_bldc_pwm_from_analog(uint16_t val);
void set_bldc_pwm_from_uart(char *val);
void bldc_loop();

#endif // BLDC_DRIVE_H