#include "bldc_drive.h"

volatile int8_t bldc_step = 0;
volatile bool is_start = true;
volatile bool is_stall = false;
int stall_count = 0;
alarm_id_t stall_alarm;
uint16_t target_pwm = 0;
uint16_t bldc_pwm = 0;
Motor_Dir bldc_dir = MOTOR_DIR_CW;
uint32_t cur_time = 0;
uint32_t prev_time = 0;

void bldc_init()
{
    #if DEBUG_EN
    printf("Setup GPIO for BEMF Detection\n");
    #endif
    gpio_init_mask(1 << sensor_A | 1 << sensor_B | 1 << sensor_C);
    gpio_set_dir_in_masked(1 << sensor_A | 1 << sensor_B | 1 << sensor_C);

    #if DEBUG_EN
    printf("Enable GPIO for PWM Generation\n");
    #endif
    gpio_init_mask(1 << phase_A_low | 1 << phase_B_low | 1 << phase_C_low);
    gpio_set_dir_out_masked(1 << phase_A_low | 1 << phase_B_low | 1 << phase_C_low);
    gpio_set_function_masked(1 << phase_A_high | 1 << phase_B_high | 1 << phase_C_high, GPIO_FUNC_PWM);
    uint slice_num_1 = pwm_gpio_to_slice_num(phase_A_high);
    uint slice_num_2 = pwm_gpio_to_slice_num(phase_B_high);
    uint slice_num_3 = pwm_gpio_to_slice_num(phase_C_high);
    pwm_set_freq(slice_num_1, 31000);
    pwm_set_freq(slice_num_2, 31000);
    pwm_set_freq(slice_num_3, 31000);
    pwm_set_output_polarity(slice_num_1, false, false);
    pwm_set_output_polarity(slice_num_2, false, false);
    pwm_set_output_polarity(slice_num_3, false, false);
    pwm_set_mask_enabled(1 << slice_num_1 | 1 << slice_num_2 | 1 << slice_num_3);
    pwm_set_gpio_level(phase_A_high, 0);
    pwm_set_gpio_level(phase_B_high, 0);
    pwm_set_gpio_level(phase_C_high, 0);

    #if DEBUG_EN
    printf("Setup BEMF Interrupt\n");
    #endif
    gpio_set_irq_enabled_with_callback(sensor_A, GPIO_IRQ_EDGE_RISE, false, update_step);
    gpio_set_irq_enabled_with_callback(sensor_A, GPIO_IRQ_EDGE_FALL, false, update_step);
    gpio_set_irq_enabled_with_callback(sensor_B, GPIO_IRQ_EDGE_RISE, false, update_step);
    gpio_set_irq_enabled_with_callback(sensor_B, GPIO_IRQ_EDGE_FALL, false, update_step);
    gpio_set_irq_enabled_with_callback(sensor_C, GPIO_IRQ_EDGE_RISE, false, update_step);
    gpio_set_irq_enabled_with_callback(sensor_C, GPIO_IRQ_EDGE_FALL, false, update_step);

    #if DEBUG_EN
    printf("Charging the Bootstrap Capacitor\n");
    #endif
    gpio_put(phase_A_low, phase_Al_high);
    gpio_put(phase_B_low, phase_Bl_high);
    gpio_put(phase_C_low, phase_Cl_high);
    sleep_ms(1000);
    gpio_put(phase_A_low, phase_Al_low);
    gpio_put(phase_B_low, phase_Bl_low);
    gpio_put(phase_C_low, phase_Cl_low);

    #if DEBUG_EN
    printf("Initialization Complete\n");
    #endif
}

int64_t stall_protection(alarm_id_t id, __unused void *user_data)
{
    #if DEBUG_EN
    printf("Stall Detected\n");
    #endif
    stall_count++;
    bldc_pwm = 0;
    cancel_alarm(stall_alarm);
}

void update_step(uint gpio, uint32_t event_mask)
{
    if (bldc_pwm > PWM_THRESH)
    {
        cancel_alarm(stall_alarm);
        stall_alarm = add_alarm_in_ms(STALL_THRESH, stall_protection, NULL, false);
    }
    if (!is_start)
    {
        bldc_step++;
        bldc_step %= 6;
        if(bldc_dir  == MOTOR_DIR_CW)
            bldc_move_cw();
        else
            bldc_move_ccw();
    }
}

void bldc_move_cw()
{
    switch (bldc_step)
    {
    case 0:
        pwm_set_gpio_level(phase_C_high, 0);
        pwm_set_gpio_level(phase_A_high, bldc_pwm);
        gpio_put(phase_B_low, phase_Bl_high);
        gpio_put(phase_A_low, phase_Al_low);
        gpio_set_irq_enabled_with_callback(sensor_A, GPIO_IRQ_EDGE_FALL, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_C, GPIO_IRQ_EDGE_RISE, true, update_step);
        break;
    case 1:
        pwm_set_gpio_level(phase_B_high, 0);
        pwm_set_gpio_level(phase_A_high, bldc_pwm);
        gpio_put(phase_C_low, phase_Cl_high);
        gpio_put(phase_B_low, phase_Bl_low);
        gpio_set_irq_enabled_with_callback(sensor_C, GPIO_IRQ_EDGE_RISE, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_B, GPIO_IRQ_EDGE_FALL, true, update_step);
        break;
    case 2:
        pwm_set_gpio_level(phase_A_high, 0);
        pwm_set_gpio_level(phase_B_high, bldc_pwm);
        gpio_put(phase_C_low, phase_Cl_high);
        gpio_put(phase_B_low, phase_Bl_low);
        gpio_set_irq_enabled_with_callback(sensor_B, GPIO_IRQ_EDGE_FALL, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_A, GPIO_IRQ_EDGE_RISE, true, update_step);
        break;
    case 3:
        pwm_set_gpio_level(phase_C_high, 0);
        pwm_set_gpio_level(phase_B_high, bldc_pwm);
        gpio_put(phase_A_low, phase_Al_high);
        gpio_put(phase_C_low, phase_Cl_low);
        gpio_set_irq_enabled_with_callback(sensor_A, GPIO_IRQ_EDGE_RISE, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_C, GPIO_IRQ_EDGE_FALL, true, update_step);
        break;
    case 4:
        pwm_set_gpio_level(phase_B_high, 0);
        pwm_set_gpio_level(phase_C_high, bldc_pwm);
        gpio_put(phase_A_low, phase_Al_high);
        gpio_put(phase_C_low, phase_Cl_low);
        gpio_set_irq_enabled_with_callback(sensor_C, GPIO_IRQ_EDGE_FALL, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_B, GPIO_IRQ_EDGE_RISE, true, update_step);
        break;
    case 5:
        pwm_set_gpio_level(phase_A_high, 0);
        pwm_set_gpio_level(phase_C_high, bldc_pwm);
        gpio_put(phase_B_low, phase_Bl_high);
        gpio_put(phase_A_low, phase_Al_low);
        gpio_set_irq_enabled_with_callback(sensor_B, GPIO_IRQ_EDGE_RISE, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_A, GPIO_IRQ_EDGE_FALL, true, update_step);
        break;
    }
}

void bldc_move_ccw()
{
    switch (bldc_step)
    {
    case 0:
        pwm_set_gpio_level(phase_A_high, 0);
        pwm_set_gpio_level(phase_C_high, bldc_pwm);
        gpio_put(phase_B_low, phase_Bl_high);
        gpio_put(phase_A_low, phase_Al_low);
        gpio_set_irq_enabled_with_callback(sensor_C, GPIO_IRQ_EDGE_FALL, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_A, GPIO_IRQ_EDGE_RISE, true, update_step);
        break;
    case 1:
        pwm_set_gpio_level(phase_B_high, 0);
        pwm_set_gpio_level(phase_C_high, bldc_pwm);
        gpio_put(phase_A_low, phase_Al_high);
        gpio_put(phase_B_low, phase_Bl_low);
        gpio_set_irq_enabled_with_callback(sensor_A, GPIO_IRQ_EDGE_RISE, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_B, GPIO_IRQ_EDGE_FALL, true, update_step);
        break;
    case 2:
        pwm_set_gpio_level(phase_C_high, 0);
        pwm_set_gpio_level(phase_B_high, bldc_pwm);
        gpio_put(phase_A_low, phase_Al_high);
        gpio_put(phase_C_low, phase_Cl_low);
        gpio_set_irq_enabled_with_callback(sensor_B, GPIO_IRQ_EDGE_FALL, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_C, GPIO_IRQ_EDGE_RISE, true, update_step);
        break;
    case 3:
        pwm_set_gpio_level(phase_A_high, 0);
        pwm_set_gpio_level(phase_B_high, bldc_pwm);
        gpio_put(phase_C_low, phase_Cl_high);
        gpio_put(phase_A_low, phase_Al_low);
        gpio_set_irq_enabled_with_callback(sensor_C, GPIO_IRQ_EDGE_RISE, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_A, GPIO_IRQ_EDGE_FALL, true, update_step);
        break;
    case 4:
        pwm_set_gpio_level(phase_B_high, 0);
        pwm_set_gpio_level(phase_A_high, bldc_pwm);
        gpio_put(phase_C_low, phase_Cl_high);
        gpio_put(phase_B_low, phase_Bl_low);
        gpio_set_irq_enabled_with_callback(sensor_A, GPIO_IRQ_EDGE_FALL, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_B, GPIO_IRQ_EDGE_RISE, true, update_step);
        break;
    case 5:
        pwm_set_gpio_level(phase_C_high, 0);
        pwm_set_gpio_level(phase_A_high, bldc_pwm);
        gpio_put(phase_B_low, phase_Bl_high);
        gpio_put(phase_C_low, phase_Cl_low);
        gpio_set_irq_enabled_with_callback(sensor_B, GPIO_IRQ_EDGE_RISE, false, update_step);
        gpio_set_irq_enabled_with_callback(sensor_C, GPIO_IRQ_EDGE_FALL, true, update_step);
        break;
    }
}

void set_bldc_pwm_from_analog(uint16_t val)
{
    #if BIDIR_EN
    if (val >= 2048)
    {
        target_pwm = (val - 2048) * 2;
        bldc_dir = MOTOR_DIR_CW;
    }
    else
    {
        target_pwm = (2048 - val) * 2;
        bldc_dir = MOTOR_DIR_CCW;
    }
    #else
    target_pwm = val;
    #endif
}

void set_bldc_pwm_from_uart(char *val)
{
    if (val[0] == 'S')
    {
        int value = atoi(val + 2);
        if(value > 0 && value < 4095)
        {
            target_pwm = value;
        }
        #if DEBUG_EN
        else
            printf("Invalid PWM value: %d\n", value);
        #endif
    }
    else if (val[0] == 'D')
    {
        int value = atoi(val + 2);
        if (value == 0)
            bldc_dir = MOTOR_DIR_CW;
        else if (value == 1)
            bldc_dir = MOTOR_DIR_CCW;
        #if DEBUG_EN
        else
            printf("Invalid Direction value: %d\n", val);
        #endif
    }
}

void bldc_loop()
{
    if (stall_count >= STALL_RECOVERY_ATTEMPTS)
    {
        stall_count = 0;
        is_stall = true;
        #if DEBUG_EN
        printf("Stall recovery attempts maxed out. Reset ESC.\n");
        #endif
    }
    else if (bldc_pwm < PWM_THRESH)
    {
        pwm_set_gpio_level(phase_A_high, 0);
        pwm_set_gpio_level(phase_B_high, 0);
        pwm_set_gpio_level(phase_C_high, 0);
        gpio_put(phase_A_low, phase_Al_low);
        gpio_put(phase_B_low, phase_Bl_low);
        gpio_put(phase_C_low, phase_Cl_low);
        is_start = true;
    }
    else if (is_start)
    {
        for(int i = 0; i < 1200; i++)
        {
            sleep_us(OPEN_LOOP_TIME);
            bldc_step++;
            bldc_step %= 6;
            #if BIDIR_EN
            if (dir == MOTOR_DIR_CW)
                bldc_move_cw();
            else
                bldc_move_ccw();
            #else
            bldc_move_cw();
            #endif
        }
        is_start = false;
    }

    cur_time = time_us_32();
    if (cur_time - prev_time >= 10000)
    {
        prev_time = cur_time;
        if (bldc_pwm < target_pwm)
            bldc_pwm += 1;
        else if (bldc_pwm > target_pwm)
            bldc_pwm -= 1;
    }
}