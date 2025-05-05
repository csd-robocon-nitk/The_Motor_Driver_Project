#ifndef CONFIG_H
#define CONFIG_H

// Set to BLDC drive mode
#define BLDC_MODE 1
// Set to BDC drive mode
#define BDC_MODE 0
// Note: Only one of the above modes should be set to 1

// Enable debug data
#define DEBUG_EN 0

// Enable Bi-directional control (for Analog Input Mode)
#define BIDIR_EN 0

// PWM signal cutoff threshold (0 - 4095) (for BLDC Mode)
#define PWM_THRESH 1000

// Open loop timing at PWM cutoff (for BLDC Mode)
#define OPEN_LOOP_TIME 3675

// Stall detection threshold (in ms) (for BLDC Mode)
#define STALL_THRESH 5

// Stall recovery attempts (for BLDC Mode)
#define STALL_RECOVERY_ATTEMPTS 3

// Signal Input Mode
#define ANALOG_INPUT 1
#define UART_INPUT 0
#define USB_SERIAL_INPUT 0
// Note: Only one of the above modes should be set to 1.

// PID Control Enable (for BDC Mode in UART or USB serial Input Mode)
#define PID_EN 0

// PID Update Interval (in ms) (for BDC Mode in UART or USB serial Input Mode)
#define PID_INTERVAL 10

// PID Control Parameters (for BDC Mode in UART or USB serial Input Mode)
#define KP 0.1
#define KI 0.01
#define KD 0.01
#define MAX_SPEED 1000
#define ENC_PPR 100

#endif // CONFIG_H