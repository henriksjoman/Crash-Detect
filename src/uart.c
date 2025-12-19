
#include <zephyr/drivers/uart.h>
#include <zephyr/devicetree.h>
#include <stdio.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include "uart.h"


#define RECEIVE_BUFF_SIZE 10
#define RECEIVE_TIMEOUT 100

const struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
	};

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));
static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};

bool green_led_state = false;
bool red_led_state = false;
bool yellow_led_state = false;
	
static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) {

	case UART_RX_RDY:
		if ((evt->data.rx.len) == 1) {
			if (evt->data.rx.buf[evt->data.rx.offset] == '0') {
				red_led_state = !red_led_state;
			} else if (evt->data.rx.buf[evt->data.rx.offset] == '1') {
				green_led_state = !green_led_state;
			} else if (evt->data.rx.buf[evt->data.rx.offset] == '2') {
				yellow_led_state = !yellow_led_state;
			}
		}
		break;
	case UART_RX_DISABLED:
		uart_rx_enable(dev, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
		break;

	default:
		break;
	}
}

void uart_send(uint32_t mom_current)
{
   	static char tx_buf[20];
    int len = snprintf(tx_buf, sizeof(tx_buf), ":%d\r\n", mom_current);
    uart_tx(uart, tx_buf, len, SYS_FOREVER_US);
 	//snprintf(tx_buf, sizeof(tx_buf), "uA:%d\r\n", mom_current);
	//uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
}

void uart_init(void)
{
   uart_configure(uart, &uart_cfg);
   uart_callback_set(uart, uart_cb, NULL);
   uart_rx_enable(uart ,rx_buf,sizeof rx_buf,RECEIVE_TIMEOUT);
}