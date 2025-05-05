#ifndef PINMAP_H
#define PINMAP_H

// Motor Coil Pins
#define phase_A_high 11
#define phase_A_low 10
#define phase_B_high 13
#define phase_B_low 12
#define phase_C_high 15
#define phase_C_low 14

// BEMF Pins
#define sensor_A 18
#define sensor_B 19
#define sensor_C 20

// GPIO Pin States
#define phase_Al_low  1
#define phase_Al_high 0
#define phase_Bl_low  0
#define phase_Bl_high 1
#define phase_Cl_low  1
#define phase_Cl_high 0

// Encoder Pins
#define encoder_A 8
#define encoder_B 9

// UART Pins
#define uart_tx 0
#define uart_rx 1

// Analog Control Pins
#define signal_in 27

#endif  // PINMAP_H