#include <stdio.h>

#include "config.h"
#include "utilities.h"
#include "bldc_drive.h"

#if BLDC_MODE && BDC_MODE
#error "BLDC_MODE and BDC_MODE cannot be both set to 1"
#endif
#if !BLDC_MODE && !BDC_MODE
#error "Either one of BLDC_MODE or BDC_MODE must be set to 1"
#endif

#if ANALOG_INPUT && UART_INPUT && USB_SERIAL_INPUT
#error "Only one of ANALOG_INPUT, UART_INPUT, or USB_SERIAL_INPUT can be set to 1"
#endif
#if !ANALOG_INPUT && !UART_INPUT && !USB_SERIAL_INPUT
#error "Either one of ANALOG_INPUT, UART_INPUT, or USB_SERIAL_INPUT must be set to 1"
#endif

int main()
{
    #if DEBUG_EN || USB_SERIAL_INPUT
    stdio_init_all();
    #endif

    #if ANALOG_INPUT
    analog_signal_init();
    #endif

    #if UART_INPUT
    uart_signal_init();
    #endif

    #if BLDC_MODE
    bldc_init();
    while (!is_stall)
    {
        #if ANALOG_INPUT
        set_bldc_pwm_from_analog(read_analog_signal());
        #endif
        #if UART_INPUT
        char *uart_val = read_uart_signal();
        set_bldc_pwm_from_uart(uart_val);
        free(uart_val);
        #endif
        #if USB_SERIAL_INPUT
        char *usb_val = read_usb_serial_signal();
        set_bldc_pwm_from_uart(usb_val);
        free(usb_val);
        #endif
        bldc_loop();
    }
    #endif

    #if DEBUG_EN
    printf(":( Restart ESC\n");
    #endif
    while (true);
    return 0;
}