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

/*
 * WHAT: Declares the I2C operations any sensor driver in this repo is
 * allowed to use: read some registers, write one register.
 *
 * HOW: This is a "port" in hexagonal/ports-and-adapters architecture
 * terms: an abstract interface with no hardware knowledge in it at all,
 * just function signatures and a status enum. Two different .c files
 * "adapt" this same interface to two different worlds: real KL26Z hardware
 * (hal_i2c_kl26z.c) and a host-native test double (mock_hal_i2c.c).
 *
 * WHY: This is the entire point of P4-C's architecture: drivers/
 * accel_fxos8700.c, the code that actually knows the accelerometer's
 * register map, never includes fsl_i2c.h or touches ARM-specific code at
 * all, it only calls HAL_I2C_ReadRegs/HAL_I2C_WriteReg. That's what makes
 * it possible to compile the IDENTICAL driver source file twice: once for
 * real hardware (app/), once linked against a plain-array fake for
 * automated host-side unit tests (tests/), with zero code duplication or
 * #ifdef branching inside the driver itself.
 */
#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdint.h>

/*
 * WHAT: The possible outcomes of an I2C operation, kept deliberately
 * hardware-agnostic.
 * WHY: kHalI2cErrorNack and kHalI2cErrorTimeout are named for what
 * happened from the CALLER's point of view (a device didn't answer, or the
 * bus hung), not in terms of any particular chip's status register bits;
 * that's what lets the mock implementation report these same statuses
 * without needing to fake real SDK error codes.
 */
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
