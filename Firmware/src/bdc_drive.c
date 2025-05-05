#include "bdc_drive.h"

int bdc_pwm = 0;
Motor_Dir bdc_dir = MOTOR_DIR_CW;

#if PID_EN
struct repeating_timer pid_timer;
int enc_count = 0;
float kp = KP;
float ki = KI;
float kd = KD;
float max_speed = MAX_SPEED;
float target_speed = 0;
float current_speed = 0;
float prev_error = 0;
float integral = 0;
#endif

void bdc_init() {
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
    printf("Charging the Bootstrap Capacitor\n");
    #endif
    gpio_put(phase_A_low, phase_Al_high);
    gpio_put(phase_B_low, phase_Bl_high);
    gpio_put(phase_C_low, phase_Cl_high);
    sleep_ms(1000);
    gpio_put(phase_A_low, phase_Al_low);
    gpio_put(phase_B_low, phase_Bl_low);
    gpio_put(phase_C_low, phase_Cl_low);

    #if PID_EN
    #if DEBUG_EN
    printf("Setup GPIO for Encoder\n");
    #endif
    gpio_init_mask(1 << encoder_A | 1 << encoder_B);
    gpio_set_dir_in_masked(1 << encoder_A | 1 << encoder_B);

    #if DEBUG_EN
    printf("Setup Encoder Interrupt\n");
    #endif
    gpio_set_irq_enabled_with_callback(encoder_A, GPIO_IRQ_EDGE_RISE, true, update_tick);
    add_repeating_timer_ms(PID_INTERVAL, pid_update, NULL, &pid_timer);
    #endif

    #if DEBUG_EN
    printf("Initialization Complete\n");
    #endif
}

#if PID_EN
void update_tick(uint gpio, uint32_t event_mask) {
    if (gpio_get(encoder_B) == 0) {
            enc_count++;
    } else {
        enc_count--;
    }
}

bool pid_update(__unused struct repeating_timer *t) {
    float error = target_speed - (enc_count / ENC_PPR) * (60000.0 / PID_INTERVAL);
    integral += error * PID_INTERVAL / 1000.0;
    float derivative = (error - prev_error) / (PID_INTERVAL / 1000.0);
    bdc_pwm = kp * error + ki * integral + kd * derivative;

    if (bdc_pwm >= 0)
        bdc_dir = MOTOR_DIR_CW;
    else
        bdc_dir = MOTOR_DIR_CCW;
    bdc_pwm = abs(bdc_pwm);
    if (bdc_pwm > 4095)
        bdc_pwm = 4095;
    prev_error = error;
    enc_count = 0;

    return true;
}
#endif

void bdc_move()
{
    if (bdc_dir == MOTOR_DIR_CW)
    {
        pwm_set_gpio_level(phase_A_high, bdc_pwm);
        pwm_set_gpio_level(phase_B_high, 0);
        gpio_put(phase_B_low, phase_Bl_high);
        gpio_put(phase_A_low, phase_Al_low);
    }
    else
    {
        pwm_set_gpio_level(phase_B_high, bdc_pwm);
        pwm_set_gpio_level(phase_A_high, 0);
        gpio_put(phase_A_low, phase_Al_high);
        gpio_put(phase_B_low, phase_Bl_low);
    }
}

void set_bdc_pwm_from_analog(uint16_t val)
{
    #if BIDIR_EN
    if (val >= 2048)
    {
        bdc_pwm = (val - 2048) * 2;
        bdc_dir = MOTOR_DIR_CW;
    }
    else
    {
        bdc_pwm = (2048 - val) * 2;
        bdc_dir = MOTOR_DIR_CCW;
    }
    #else
    bdc_pwm = val;
    #endif
}

void set_bdc_pwm_from_uart(char *val)
{
    if (val[0] == 'S')
    {
        #if PID_EN
        target_speed = atoi(val + 2);
        if (target_speed > MAX_SPEED)
            target_speed = MAX_SPEED;
        else if (target_speed < -MAX_SPEED)
            target_speed = -MAX_SPEED;
        #if DEBUG_EN
        printf("Target Speed: %d\n", target_speed);
        #endif
        #else
        int value = atoi(val + 2);
        if(value > 0 && value < 4095)
        {
            bdc_pwm = value;
        }
        #if DEBUG_EN
        else
            printf("Invalid PWM value: %d\n", value);
        #endif
        #endif
    }
    #if PID_EN == 0
    else if (val[0] == 'D')
    {
        int value = atoi(val + 2);
        if (value == 0)
            bdc_dir = MOTOR_DIR_CW;
        else if (value == 1)
            bdc_dir = MOTOR_DIR_CCW;
        #if DEBUG_EN
        else
            printf("Invalid Direction value: %d\n", val);
        #endif    
    }
    #endif
    #if PID_EN
    else if (val[0] == 'P')
    {
        kp = atof(val + 2);target_speed = atoi(val + 2);
        #if DEBUG_EN
        printf("New Kp: %f\n", kp);
        #endif
    }
    else if (val[0] == 'I')
    {
        ki = atof(val + 2);
        #if DEBUG_EN
        printf("New Ki: %f\n", ki);
        #endif
    }
    else if (val[0] == 'D')
    {
        kd = atof(val + 2);
        #if DEBUG_EN
        printf("New Kd: %f\n", kd);
        #endif
    }
    #endif
}
