#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include "acc.h"
#include "acc_spi.h"

int set_acc_sleep_state(void) // Low power setup for wake up on any movement.
{
  int err = 0;
  err = write_acc_spi(CTRL_REG1, (ODR10HZ | 0x0f));	// 10Hz sample rate, all axises enables
  if (err != 0)
  {
    return err;
  }
  err = write_acc_spi(CTRL_REG2, 0x02);	// Enable HP filter for interrupt 2. Cut off freq. = 0.2 Hz
  if (err != 0)
  {
	  return err;
  }
  err = write_acc_spi(CTRL_REG6, (0x20));	// enable interrupt on int2 pin
  if (err != 0)
  {
	  return err;
  }
  err = write_acc_spi(CTRL_REG4, (0x00 | RANGE_2G));	// set range to +-2g
  if (err != 0)
  {
	  return err;
  }
  err = write_acc_spi(CTRL_REG5, (0x02)); // enable latch interrupt. Clear interrupt by reading INT2_SRC
  if (err != 0)
  {
	  return err;
  }
  k_msleep(1);
  err = write_acc_spi(INT2_TH, 0x0f);		// g-force interrupt threshold 0.24g
  if (err != 0)
  {
	  return err;
  }
  err = write_acc_spi(INT2_CFG, 0x2A);	        // Enable interrupt on high acc. on X, Y and Z.
  if (err != 0)
  {
	  return err;
  }
  err = read_acc_reg(REFERENCE);
  if (err != 0)
  {
	  return err;
  }
  return 0;
  
}


/* Active state. Set enough sample rate to detect a crash in time (100HZ)
and enable an interrupt that trigger on a force bigger than <tbd> G */

int set_acc_active_state(void)
{
  int err = 0;
  err = write_acc_spi(CTRL_REG1, (ODR100HZ | 0x09));	// 100Hz sample rate, X axis enabled
  if (err != 0)
  {
  return err;
  }
  k_msleep(1);
  
  err = write_acc_spi(CTRL_REG6, (0x20));	// enable force interrupt on int 2 pin
  if (err != 0)
  {
	return err;
  }
  err = write_acc_spi(CTRL_REG3, (0x00));	// disable interrupts on int 1 pin
  err = write_acc_spi(INT2_CFG, (0x02));	// enable force interrupt on pin 2 for both axises of X.
  if (err != 0)
  {
	return err;
  }
  k_msleep(1);
  err = write_acc_spi(CTRL_REG4, (0x00 | RANGE_8G));	// set range to +-8g for initial "spin test"
  if (err != 0)
  {
	return err;
  }
  err = write_acc_spi(INT2_TH, (0x2F));	// set threshold to 4G.
  if (err != 0)
  {
	return err;
  }
  k_msleep(1);
  err = write_acc_spi(CTRL_REG5, (0x48)); // Fifo enable
  if (err != 0)
  {
	return err;
  }
  k_msleep(1);
  err = write_acc_spi(FIFO_CTRL_REG, 0x80);		// fifo stream mode
  if (err != 0)
  {
  	return err;
  }
  k_msleep(1);    
  read_acc_reg(FIFO_SRC_REG);
  return 0;
}

/* If a crash occur, set a high sample rate (5376 Hz) and enable water mark interrupt 
to make sure to capture all samples */

int set_acc_crash_state(void)
{
  int err = 0;
  err = write_acc_spi(CTRL_REG6, (0x00));	// disable force interrupt on int 2 pin
  if (err != 0)
  {
	return err;
  }
   err = write_acc_spi(CTRL_REG3, (0x04));	// enable water mark interrupt on int 1 pin
  if (err != 0)
  {
	  return err;
  }
  err = write_acc_spi(FIFO_CTRL_REG, 0x8F);		// fifo stream mode and set watermark level to 15 samples
  if (err != 0)
  {
  	return err;
  }
  err = write_acc_spi(CTRL_REG1, (ODR5376HZ | 0x09));	// 5376Hz sample rate, X axis enabled
  if (err != 0)
  {
    return err;
  }
  return 0;
 }