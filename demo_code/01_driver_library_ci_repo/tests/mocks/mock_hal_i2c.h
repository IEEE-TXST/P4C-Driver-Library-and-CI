/*
 * Test-only control surface for mock_hal_i2c.c. Tests call these to set
 * up what the "hardware" returns before calling driver code, and to
 * check what the driver wrote afterward. Real firmware never sees this
 * header; only tests/ includes it.
 */
#ifndef MOCK_HAL_I2C_H
#define MOCK_HAL_I2C_H

#include <stdint.h>
#include "hal_i2c.h"

/* Clears all programmed responses and recorded writes. Call at the start
   of every test. */
void MockHalI2c_Reset(void);

/* The next HAL_I2C_ReadRegs() call for this exact (deviceAddr, reg) pair
   returns these bytes and kHalI2cOk. */
void MockHalI2c_SetReadResponse(uint8_t deviceAddr, uint8_t reg, const uint8_t *data, uint32_t len);

/* Forces every HAL_I2C_ReadRegs() call to return this status instead of
   kHalI2cOk, regardless of address/register, until the next Reset(). Used
   to simulate a device that never responds. */
void MockHalI2c_ForceReadStatus(hal_i2c_status_t status);

/* How many times HAL_I2C_WriteReg() has been called since the last Reset(). */
uint32_t MockHalI2c_GetWriteCount(void);

/* The (deviceAddr, reg, value) of the Nth write (0-indexed), for checking
   that a driver wrote the correct configuration sequence. */
void MockHalI2c_GetWrite(uint32_t index, uint8_t *deviceAddr, uint8_t *reg, uint8_t *value);

#endif /* MOCK_HAL_I2C_H */
