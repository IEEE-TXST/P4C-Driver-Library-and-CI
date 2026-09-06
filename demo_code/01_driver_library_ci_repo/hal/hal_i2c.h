/*
 * HAL I2C port: the boundary between portable driver code (drivers/) and
 * the actual hardware. Every function here has exactly two implementations
 * in this repository:
 *
 *   - hal_i2c_kl26z.c: the real one, wraps the SDK's I2C0 driver, compiled
 *     only into app/ (the real firmware, built for ARM).
 *   - tests/mocks/mock_hal_i2c.c: a fake one, plain C variables standing in
 *     for register state, compiled only into the host-native test builds.
 *
 * drivers/accel_fxos8700.c calls only these functions, never anything
 * chip-specific, which is what lets the exact same driver source file be
 * compiled unmodified for both. See the manual, Section 6.
 */
#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdint.h>

typedef enum
{
    kHalI2cOk = 0,
    kHalI2cErrorNack,   /* the device at this address didn't respond */
    kHalI2cErrorTimeout /* the bus never became idle / transfer never completed */
} hal_i2c_status_t;

/* Reads "len" bytes starting at register "reg" from the device at
   "deviceAddr" (7-bit address) into "data". */
hal_i2c_status_t HAL_I2C_ReadRegs(uint8_t deviceAddr, uint8_t reg, uint8_t *data, uint32_t len);

/* Writes a single byte "value" to register "reg" on the device at
   "deviceAddr". */
hal_i2c_status_t HAL_I2C_WriteReg(uint8_t deviceAddr, uint8_t reg, uint8_t value);

#endif /* HAL_I2C_H */
