#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#define BEMF_EN 1

// Low is active low
// High is active high
#define A_H 10
#define A_L 11
#define B_H 12
#define B_L 13
#define C_H 14
#define C_L 15

#define sense_A 18
#define sense_B 19
#define sense_C 20

uint16_t pwm = 2000; //4050 max
volatile uint8_t bldc_step = 0;
uint32_t global_wrap = 0;
bool is_start = true;
int j = 0;

uint32_t pwm_set_freq(uint slice_num, uint32_t f)
{
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / f / 4096 + (clock % (f * 4096) != 0);
    if (divider16 / 16 == 0)
        divider16 = 16;
    uint32_t wrap = clock * 16 / divider16 / f - 1;
    pwm_set_clkdiv_int_frac(slice_num, divider16 / 16, divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    return wrap;
}

void bldc_move();

void update_step(uint gpio, uint32_t event_mask)
{
    gpio_put(25, bldc_step % 2);
    if (!is_start | j >= 3599)
    {
        bldc_step++;
        bldc_step %= 6;
        bldc_move();
    }
}

// BLDC motor commutation function
void bldc_move()
{
    switch (bldc_step)
    {
    case 0:
        pwm_set_gpio_level(C_H, 0);
        pwm_set_gpio_level(A_H, pwm);
        gpio_put(B_L, 0);
        gpio_put(A_L, 1);
        #if BEMF_EN
        gpio_set_irq_enabled_with_callback(sense_A, GPIO_IRQ_EDGE_FALL, false, update_step);
        gpio_set_irq_enabled_with_callback(sense_C, GPIO_IRQ_EDGE_RISE, true, update_step);
        #endif
        break;
    case 1:
        pwm_set_gpio_level(B_H, 0);
        pwm_set_gpio_level(A_H, pwm);
        gpio_put(C_L, 0);
        gpio_put(B_L, 1);
        #if BEMF_EN
        gpio_set_irq_enabled_with_callback(sense_C, GPIO_IRQ_EDGE_RISE, false, update_step);
        gpio_set_irq_enabled_with_callback(sense_B, GPIO_IRQ_EDGE_FALL, true, update_step);
        #endif
        break;
    case 2:
        pwm_set_gpio_level(A_H, 0);
        pwm_set_gpio_level(B_H, pwm);
        gpio_put(C_L, 0);
        gpio_put(B_L, 1);
        #if BEMF_EN
        gpio_set_irq_enabled_with_callback(sense_B, GPIO_IRQ_EDGE_FALL, false, update_step);
        gpio_set_irq_enabled_with_callback(sense_A, GPIO_IRQ_EDGE_RISE, true, update_step);
        #endif
        break;
    case 3:
        pwm_set_gpio_level(C_H, 0);
        pwm_set_gpio_level(B_H, pwm);
        gpio_put(A_L, 0);
        gpio_put(C_L, 1);
        #if BEMF_EN
        gpio_set_irq_enabled_with_callback(sense_A, GPIO_IRQ_EDGE_RISE, false, update_step);
        gpio_set_irq_enabled_with_callback(sense_C, GPIO_IRQ_EDGE_FALL, true, update_step);
        #endif
        break;
    case 4:
        pwm_set_gpio_level(B_H, 0);
        pwm_set_gpio_level(C_H, pwm);
        gpio_put(A_L, 0);
        gpio_put(C_L, 1);
        #if BEMF_EN
        gpio_set_irq_enabled_with_callback(sense_C, GPIO_IRQ_EDGE_FALL, false, update_step);
        gpio_set_irq_enabled_with_callback(sense_B, GPIO_IRQ_EDGE_RISE, true, update_step);
        #endif
        break;
    case 5:
        pwm_set_gpio_level(A_H, 0);
        pwm_set_gpio_level(C_H, pwm);
        gpio_put(B_L, 0);
        gpio_put(A_L, 1);
        #if BEMF_EN
        gpio_set_irq_enabled_with_callback(sense_B, GPIO_IRQ_EDGE_RISE, false, update_step);
        gpio_set_irq_enabled_with_callback(sense_A, GPIO_IRQ_EDGE_FALL, true, update_step);
        #endif
        break;
    }
}

int main()
{
    // BEMF
    gpio_init_mask(1 << sense_A | 1 << sense_B | 1 << sense_C);
    gpio_set_dir_in_masked(1 << sense_A | 1 << sense_B | 1 << sense_C);
    // gpio_set_pulls(sense_A, true, false);
    // gpio_set_pulls(sense_B, true, false);
    // gpio_set_pulls(sense_C, true, false);

    // PWM  
    gpio_init_mask(1 << A_L | 1 << B_L | 1 << C_L);
    gpio_init_mask(1 << 25);
    gpio_set_dir_out_masked(1 << A_L | 1 << B_L | 1 << C_L | 1 << 25);
    gpio_set_function_masked(1 << A_H | 1 << B_H | 1 << C_H, GPIO_FUNC_PWM);
    uint slice_num_1 = pwm_gpio_to_slice_num(A_H);
    uint slice_num_2 = pwm_gpio_to_slice_num(B_H);
    uint slice_num_3 = pwm_gpio_to_slice_num(C_H);
    global_wrap = pwm_set_freq(slice_num_1, 31000);
    pwm_set_freq(slice_num_2, 31000);
    pwm_set_freq(slice_num_3, 31000);
    pwm_set_output_polarity(slice_num_1, false, false);
    pwm_set_output_polarity(slice_num_2, false, false);
    pwm_set_output_polarity(slice_num_3, false, false);
    pwm_set_mask_enabled(1 << slice_num_1 | 1 << slice_num_2 | 1 << slice_num_3);

    // stdio_init_all();
    // adc_init();
    // adc_gpio_init(26);
    // adc_select_input(0);
    // printf("%d", global_wrap);

    // GPIO Interrupt
    #if BEMF_EN
    gpio_set_irq_enabled_with_callback(sense_A, GPIO_IRQ_EDGE_RISE, false, update_step);
    gpio_set_irq_enabled_with_callback(sense_A, GPIO_IRQ_EDGE_FALL, false, update_step);
    gpio_set_irq_enabled_with_callback(sense_B, GPIO_IRQ_EDGE_RISE, false, update_step);
    gpio_set_irq_enabled_with_callback(sense_B, GPIO_IRQ_EDGE_FALL, false, update_step);
    gpio_set_irq_enabled_with_callback(sense_C, GPIO_IRQ_EDGE_RISE, false, update_step);
    gpio_set_irq_enabled_with_callback(sense_C, GPIO_IRQ_EDGE_FALL, false, update_step);
    #endif

    gpio_put(A_L, 0);
    gpio_put(B_L, 0);
    gpio_put(C_L, 0);
    sleep_ms(2000);
    gpio_put(A_L, 1);
    gpio_put(B_L, 1);
    gpio_put(C_L, 1);
    // gpio_put(25, 1);

    is_start = true;
    // uint16_t val = 2048;
    while (true)
    {
        
        // motor starter
        if (is_start)
        {
            int i = 3645;
            // while (i > 1500)
            // {
            //     sleep_us(i);
            //     bldc_move();
            //     bldc_step++;
            //     bldc_step %= 6;
            //     i = i - 4;
            //     pwm = pwm + 5;
            // }
            // is_start = false;
            for(j = 0; j < 3600; j++)
            {
                // val = adc_read();
                sleep_us(i);
                bldc_step++;
                bldc_step %= 6;
                bldc_move();
                // printf("%d\n", i);
                // i = 5500 + (val - 2048);
            }
            is_start = false;
        }
    }
    return 0;
}