#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "acc_spi.h"
#include "acc.h"
#include "uart.h"

#define LED_NODE_1 DT_ALIAS(led3)
#define LED_NODE_2 DT_ALIAS(led4)
#define LED_NODE_3 DT_ALIAS(led5)
#define LED_NODE_4 DT_ALIAS(led0)
#define LED_NODE_5 DT_ALIAS(led1)
#define LED_NODE_6 DT_ALIAS(led2)
#define INT_1_NODE DT_ALIAS(digin1)
#define INT_2_NODE DT_ALIAS(digin2)

static const struct gpio_dt_spec s_led_1 = GPIO_DT_SPEC_GET(LED_NODE_1, gpios);
static const struct gpio_dt_spec s_led_2 = GPIO_DT_SPEC_GET(LED_NODE_2, gpios);
static const struct gpio_dt_spec s_led_3 = GPIO_DT_SPEC_GET(LED_NODE_3, gpios);
static const struct gpio_dt_spec s_led_4 = GPIO_DT_SPEC_GET(LED_NODE_4, gpios);
static const struct gpio_dt_spec s_led_5 = GPIO_DT_SPEC_GET(LED_NODE_5, gpios);
static const struct gpio_dt_spec s_led_6 = GPIO_DT_SPEC_GET(LED_NODE_6, gpios);
static const struct gpio_dt_spec fifo_overrun_int = GPIO_DT_SPEC_GET(INT_1_NODE, gpios);
static const struct gpio_dt_spec force_int = GPIO_DT_SPEC_GET(INT_2_NODE, gpios);
static struct gpio_callback s_sensor_cb_data;
static struct gpio_callback s_sensor2_cb_data;

static const struct gpio_dt_spec *led_ptrs[] = {
    &s_led_1, &s_led_2, &s_led_3, &s_led_4, &s_led_5, &s_led_6
};

enum accelerometer_state {
  SLEEP_STATE, ACTIVE_STATE, CRASH_STATE
};

volatile bool g_state_changed = false;
volatile bool g_read_fifo = false;
volatile bool g_abort_detection = false;
uint8_t g_old_acc = 0;
uint8_t g_fifo_counter = 0;

enum accelerometer_state g_current_state = SLEEP_STATE;

/* ***** */
static void timer_expiry(struct k_timer *timer);
void start_1min_timer(void);
static void fifo_interrupt(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
static void force_interrupt(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
static void crash_confirmed(void);
static void fifo_drain(void);

/* ***** */

K_TIMER_DEFINE(countdown_timer, timer_expiry, NULL);

static void timer_expiry(struct k_timer *timer)
{
    g_current_state =  SLEEP_STATE;
    g_state_changed = true;
}

void start_1min_timer(void)
{
    k_timer_start(&countdown_timer,
                  K_SECONDS(60),
                  K_NO_WAIT);
}

static void fifo_interrupt(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
  g_read_fifo = true;
}

static void force_interrupt(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
  if (g_current_state == SLEEP_STATE)
  {
    g_current_state = ACTIVE_STATE;
    g_state_changed = true;
  } 
  else if (g_current_state == ACTIVE_STATE)
  {
    g_current_state = CRASH_STATE;
    g_state_changed = true;
    g_abort_detection = false;
  }
}

static void crash_confirmed(void)
{
  g_abort_detection = true;
  g_fifo_counter = 0;

  for (int i=0;i<10;i++)
  {
    gpio_pin_toggle_dt(led_ptrs[0]);
    gpio_pin_toggle_dt(led_ptrs[1]);
    gpio_pin_toggle_dt(led_ptrs[2]);
    gpio_pin_toggle_dt(led_ptrs[3]);
    gpio_pin_toggle_dt(led_ptrs[4]);
    gpio_pin_toggle_dt(led_ptrs[5]);
    k_msleep(200);
  }
  g_current_state = ACTIVE_STATE;
  g_state_changed = true;
}

static void fifo_drain(void)
{
  int8_t acc = 0;
  while (!(read_acc_reg(0x2f) & 0x20) && !g_abort_detection)
  {
    acc = (int8_t)read_acc_reg(0x29);
    if (g_fifo_counter > 0)
    {
      if ((acc < 0) != (g_old_acc < 0))
      {
        g_abort_detection = true;
        g_fifo_counter = 0;
        set_acc_active_state();
        g_current_state = ACTIVE_STATE;
      }
    }
    if (abs(acc) < 15)
    {
      g_abort_detection = true;
      g_fifo_counter = 0;
      set_acc_active_state();
      g_current_state = ACTIVE_STATE;
    }
    g_old_acc = acc;
    if (!g_abort_detection)
    {  
      g_fifo_counter++;
      if (g_fifo_counter >= 200)
      {
        crash_confirmed();
      }
    }
  }
}


int main(void)
{
  //uart_init();
  gpio_pin_configure_dt(&s_led_1, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&s_led_2, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&s_led_3, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&s_led_4, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&s_led_5, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&s_led_6, GPIO_OUTPUT_INACTIVE);
  gpio_pin_configure_dt(&fifo_overrun_int, GPIO_INPUT);
  gpio_pin_configure_dt(&force_int, GPIO_INPUT);
  gpio_pin_interrupt_configure_dt(&fifo_overrun_int, GPIO_INT_EDGE_RISING);
  gpio_pin_interrupt_configure_dt(&force_int, GPIO_INT_EDGE_RISING);
  gpio_init_callback(&s_sensor_cb_data, fifo_interrupt, (BIT(fifo_overrun_int.pin)));
  gpio_add_callback(fifo_overrun_int.port, &s_sensor_cb_data);
  gpio_init_callback(&s_sensor2_cb_data, force_interrupt, (BIT(force_int.pin)));
  gpio_add_callback(force_int.port, &s_sensor2_cb_data);
  init_spi0();
  k_msleep(1);
  if(read_acc_reg(0x0f) == 0x33)  // Read chip ID to confirm SPI function
  {
    gpio_pin_set_dt(led_ptrs[5],1);
    k_msleep(500);
    gpio_pin_set_dt(led_ptrs[5],0);
  }
  g_current_state = SLEEP_STATE;
  g_state_changed = true;
 
  k_msleep(1);

  while(1)
  {
    while (g_current_state == SLEEP_STATE)
    {
      if (g_state_changed)
      {
        g_state_changed = false;
        read_acc_reg(0x35);
        set_acc_sleep_state();
      }
      k_msleep(1000); // Demonstrate the application in sleep state.
      gpio_pin_toggle_dt(led_ptrs[0]);
    }
    
    if(g_current_state == CRASH_STATE)   // if the fifo has filled to the water mark, read the fifo until empty
    {
      if (g_state_changed)
      {
        g_state_changed = false;
        set_acc_crash_state(); // if a high G event happended, start high speed logging
      }
      fifo_drain();   
    }

    if(g_current_state == ACTIVE_STATE)
    {
      if (g_state_changed)
      {
        g_state_changed = false;
        read_acc_reg(0x35);
        start_1min_timer();
        set_acc_active_state(); 
      }
      k_msleep(300); // Demonstrate the application in sleep state.
      gpio_pin_toggle_dt(led_ptrs[1]);
      // Do normal stuff
    }
  }
  return 0;
}
