/*
 * =====================================================================================
 *
 *       Filename:  imu.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/07/2013 13:31:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef _IMU_H_
#define _IMU_H_

#include <p24FJ128GB206.h>
#include "pin.h"
#include "timer.h"
#include "spi.h"


// -------------- ACCELEROMETER -------------------- //
/* Accelerometer register addresses */
#define XOUTL   0x00
#define XOUTH   0x01
#define YOUTL   0x02
#define YOUTH   0x03
#define ZOUTL   0x04
#define ZOUTH   0x05
#define XOUT8   0x06
#define YOUT8   0x07
#define ZOUT8   0x08
#define STATUS  0x09
#define DETSRC  0x0A
#define TOUT    0x0B
#define I2CADD  0x0D
#define USRINF  0x0E
#define WHOAMI  0x0F
#define XOFFL   0x10
#define XOFFH   0x11
#define YOFFL   0x12
#define YOFFH   0x13
#define ZOFFL   0x14
#define ZOFFH   0x15
#define MCTL    0x16
#define INTRST  0x17
#define CTL1    0x18
#define CTL2    0x19
#define LDTH    0x1A
#define PDTH    0x1B
#define PW      0x1C
#define LT      0x1D
#define TW      0x1E

#define STANDBY_MODE        0b00000000
#define MEASURE_MODE        0b00000001
#define LEVEL_DETECT_MODE   0b00000010
#define PULSE_DETECT_MODE   0b00000011

#define RANGE_8G            0b00000000
#define RANGE_2G            0b00000100
#define RANGE_4G            0b00001000

// -------------- GYROSCOPE -------------------- //
// Gyro register addresses
#define WHO_AM_I       0x0F
#define CTRL_REG1      0x20
#define CTRL_REG2      0x21
#define CTRL_REG3      0x22
#define CTRL_REG4      0x23
#define CTRL_REG5      0x24
#define REFERENCE      0x25
#define OUT_TEMP       0x26
#define STATUS_REG     0x27
#define OUT_X_L        0x28
#define OUT_X_H        0x29
#define OUT_Y_L        0x2A
#define OUT_Y_H        0x2B
#define OUT_Z_L        0x2C
#define OUT_Z_H        0x2D
#define FIFO_CTRL_REG  0x2E
#define FIFO_SRC_REG   0x2F
#define INT1_CFG       0x30
#define INT1_SRC       0x31
#define INT1_TSH_XH    0x32
#define INT1_TSH_XL    0x33
#define INT1_TSH_YH    0x34
#define INT1_TSH_YL    0x35
#define INT1_TSH_ZH    0x36
#define INT1_TSH_ZL    0x37
#define INT1_DURATION  0x38


void imu_init(void);

// Accelerometer function prototypes
//
extern _PIN IMU_MOSI, IMU_MISO, IMU_SCK, ACCEL_CS, GYRO_CS;

extern int accel_xyz[3];
extern int accel_offsets[3];

void            accel_write(unsigned char address, unsigned char value);
unsigned char   accel_read(unsigned char address);
void            accel_set_measure_mode(void);
void            accel_read_xyz(int * data);
void            accel_calibrate(int * offsets);

// Gyroscope function prototypes
//
extern int gyro_xyz[3];

void            gyro_write(unsigned char address, unsigned char value);
unsigned char   gyro_read(unsigned char address);
void            gyro_init(void);

#endif
