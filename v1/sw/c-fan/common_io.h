#ifndef _COMMON_IO_H_
#define _COMMON_IO_H_

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif //uint8_t

#define UART_BAUDRATE 74880

// GPIO02: ESP-12x blue LED:
//#define MODULE_BLUELED_PIN LED_BUILTIN
#define MODULE_BLUELED_PIN 2

// relay/ssr output:
#define RY_FAN_PIN 5     // fan   active high
#define RY_LIGHT_PIN 15  // light active high

#define RY_FAN_PIN2 12   // fan   active low
#define RY_LIGHT_PIN2 14 // light active low

// inputs:
//#define SENSOR_MAINS_FREQ 12
#define TEMP_PIN A0

#define ONE_WIRE_BUS 13
#define IR_RECEIVER_PIN 4

#define WIFI_ENABLE_PIN 16

#endif //_COMMON_IO_H_
