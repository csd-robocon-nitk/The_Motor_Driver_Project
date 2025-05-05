#include "utilities.h"

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

void analog_signal_init()
{
    #if DEBUG_EN
    printf("Setup GPIO for ADC\n");
    #endif
    adc_init();
    adc_gpio_init(signal_in);
    adc_select_input(signal_in - 26);
}

void uart_signal_init()
{
    #if DEBUG_EN
    printf("Setup GPIO for UART\n");
    #endif
    uart_init(uart0, 115200);
    gpio_set_function(uart_tx, UART_FUNCSEL_NUM(uart0, 0));
    gpio_set_function(uart_rx, UART_FUNCSEL_NUM(uart0, 1));
}

uint16_t read_analog_signal()
{
    uint16_t adc_val = adc_read();
    #if DEBUG_EN
    printf("ADC Value: %d\n", adc_val);
    #endif
    return adc_val;
}

char* read_uart_signal()
{
    char* uart_val = (char *)malloc(6);
    uart_read_blocking(uart0, (uint8_t *)uart_val, 6);
    #if DEBUG_EN
    printf("UART Value: %s\n", uart_val);
    #endif
    return uart_val;
}

char* read_usb_serial_signal()
{
    char* usb_val = (char *)malloc(6);
    scanf("%[^\n]s", usb_val); 
    #if DEBUG_EN
    printf("USB Serial Value: %s\n", usb_val);
    #endif
    return usb_val;
}