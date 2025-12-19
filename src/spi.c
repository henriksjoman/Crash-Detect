#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include "acc_spi.h"

#define SPI1_NODE DT_NODELABEL(spi1)
#define SPIOP	SPI_WORD_SET(8) | SPI_TRANSFER_MSB | SPI_MODE_CPOL | SPI_MODE_CPHA

struct spi_dt_spec spispec = SPI_DT_SPEC_GET(DT_NODELABEL(bme280), SPIOP, 0);

bool init_spi0()
{
  int err = spi_is_ready_dt(&spispec);
  return err;
}

int read_acc_spi(uint8_t reg, uint8_t *data, uint8_t size)
{
  int err;
  uint8_t tx_buffer[1] = {(reg | 0x80)};
  struct spi_buf tx_spi_buf			= {.buf = (void *)&tx_buffer, .len = 1};
  struct spi_buf_set tx_spi_buf_set 	= {.buffers = &tx_spi_buf, .count = 1};
  struct spi_buf rx_spi_bufs 			= {.buf = data, .len = size};
  struct spi_buf_set rx_spi_buf_set	= {.buffers = &rx_spi_bufs, .count = 1};
  err = spi_transceive_dt(&spispec, &tx_spi_buf_set, &rx_spi_buf_set);
  return err;
}

int write_acc_spi(uint8_t reg, uint8_t value)
{
  int err;
  uint8_t tx_values[]={ (reg & 0x7F), value};
  struct spi_buf tx_spi_bufs[] = {
    { .buf = tx_values, .len = sizeof(tx_values) }
  };
  struct spi_buf_set spi_tx_buffer_set = {
    .buffers = tx_spi_bufs,
    .count = 1
  };
  err=spi_write_dt(&spispec, &spi_tx_buffer_set);
  return err;
}

int8_t read_acc_reg(uint8_t acc_reg)
{
  uint8_t rx_buf_ID[2] = {0,0};
  int err = read_acc_spi(acc_reg, rx_buf_ID, 2);
  if (err == 0)
  {
    return rx_buf_ID[1];
  } 
  else
  {
    return err;
  }
}
