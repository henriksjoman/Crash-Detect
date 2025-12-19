#ifndef ACCSPI_H
#define ACCSPI_H

#include <stdint.h>
#include <stdbool.h>

int8_t read_acc_reg(uint8_t acc_reg);
int read_acc_spi(uint8_t reg, uint8_t *data, uint8_t size);
bool init_spi0();
int write_acc_spi(uint8_t reg, uint8_t value);

#endif 