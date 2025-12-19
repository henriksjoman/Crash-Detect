#ifndef UART_H
#define UART_H
#include <stdint.h>
#include <stdbool.h>

extern bool green_led_state;
extern bool red_led_state;
extern bool yellow_led_state;

void uart_init(void);
void uart_send(uint32_t);

#endif 