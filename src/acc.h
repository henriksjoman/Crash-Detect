#ifndef ACC_H
#define ACC_H

#define WHO_AM_I 0x0f
#define CTRL_REG1 0x20
#define CTRL_REG2 0x21
#define CTRL_REG3 0x22
#define CTRL_REG4 0x23
#define CTRL_REG5 0x24
#define CTRL_REG6 0x25
#define STATUS_REG 0x27
#define FIFO_CTRL_REG 0x2e
#define FIFO_SRC_REG 0x2F
#define REFERENCE 0x26
#define X_OUT_L 0x28
#define X_OUT_H 0x29
#define Y_OUT_L 0x2A
#define Y_OUT_H 0x2B
#define Z_OUT_L 0x2C
#define Z_OUT_H 0x2D
#define INT1_DURATION 0x33
#define ODR5376HZ 0x09<<4
#define ODR200HZ 0x06<<4
#define ODR100HZ 0x05<<4
#define ODR50HZ 0x04<<4
#define ODR25HZ 0x03<<4
#define ODR10HZ 0x02<<4
#define ODR1HZ 0x01<<4
#define RANGE_2G 0x00<<4
#define RANGE_4G 0x01<<4
#define RANGE_8G 0x02<<4
#define RANGE_16G 0x03<<4
#define SCALE_2G 0.01575
#define SCALE_4G 0.0315
#define SCALE_8G 0.063
#define SCALE_16G 0.126
#define INT1_EN 1<<6
#define CTRL_REG0 0x1e
#define INT1_CFG 0x30
#define INT2_CFG 0x34
#define INT2_TH 0x36
#define INT1_SRC 0x31
#define INT1_TH 0x32
#define FIFO_EN 1<<7
#define INT1_LATCH 1<<6
#define BITSHIFT 0
#define BIT_RES 8
#define LATCHED_INT1 1<<3
#define FIFO_MODE_CONT 0x02<<6
#define FIFO_MODE_FIFO 0x01<<5
#define FIFO_MODE_CONT_TO_FIFO 0x03<<5

#include <stdint.h>
#include "acc.h"

int set_acc_active_state(void);
int set_acc_crash_state(void);
int set_acc_sleep_state(void);

#endif