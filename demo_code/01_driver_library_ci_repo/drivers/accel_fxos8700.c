/*
 * Portable FXOS8700CQ driver. Register map and conversion verified in P1
 * (see P1's manual, Section 12): WHO_AM_I at 0x0D returns 0xC7,
 * XYZ_DATA_CFG at 0x0E set to 0x01 selects +/-4g range (0.488 mg/LSB),
 * CTRL_REG1 at 0x2A goes 0x00 (standby) then 0x0D (200 Hz active), and
 * OUT_X_MSB at 0x01 starts a 6-byte X/Y/Z read. Every register access
 * goes through hal_i2c.h, never touches I2C0 directly.
 */

/*
 * WHAT: The accelerometer's logic (find it, configure it, read and convert
 * its output), written once and reused unchanged by both the real firmware
 * and the automated test suite.
 * HOW: Every hardware interaction goes through HAL_I2C_ReadRegs/WriteReg
 * (declared in hal_i2c.h), never a direct SDK call; this file has no idea
 * whether it's running against real I2C0 silicon or a plain-array test
 * double, and doesn't need to.
 * WHY: This is the payoff of the HAL boundary: the exact same register
 * sequence and conversion math this project already verified against real
 * hardware in P1 can now also be exercised automatically, on a developer's
 * laptop or in CI, with no board attached, by linking this file against
 * tests/mocks/mock_hal_i2c.c instead of hal_i2c_kl26z.c. See tests/
 * test_accel_fxos8700.c for exactly how that's exploited.
 */
#include <stddef.h>
#include "accel_fxos8700.h"
#include "hal_i2c.h"

#define ACCEL_WHOAMI_REG 0x0DU
#define ACCEL_WHOAMI_VALUE 0xC7U
#define ACCEL_XYZ_DATA_CFG_REG 0x0EU
#define ACCEL_CTRL_REG1 0x2AU
#define ACCEL_OUT_X_MSB_REG 0x01U

static const uint8_t kAccelAddresses[] = {0x1CU, 0x1DU, 0x1EU, 0x1FU};

/*
 * WHAT: Finds the sensor by probing its possible addresses, then
 * configures it for continuous +/-4g measurement.
 * HOW: Identical probe-and-configure sequence to every earlier project's
 * accelerometer code (P1, P4-A, P4-B), just expressed through
 * HAL_I2C_ReadRegs/WriteReg instead of a locally-defined I2C wrapper.
 * WHY: Writing handle->address = 0 / handle->found = false FIRST, before
 * the probe loop even runs, guarantees a caller that skips checking the
 * return value still gets a handle that safely reports "not found" rather
 * than one left in an undefined state.
 */
bool Accel_Init(accel_handle_t *handle)
{
    uint32_t i;

    handle->address = 0U;
    handle->found = false;

    for (i = 0U; i < sizeof(kAccelAddresses); i++)
    {
        uint8_t whoAmI = 0U;
        uint8_t candidate = kAccelAddresses[i];
        if ((HAL_I2C_ReadRegs(candidate, ACCEL_WHOAMI_REG, &whoAmI, 1U) == kHalI2cOk) &&
            (whoAmI == ACCEL_WHOAMI_VALUE))
        {
            handle->address = candidate;
            handle->found = true;
            break;
        }
    }

    if (!handle->found)
    {
        return false;
    }

    HAL_I2C_WriteReg(handle->address, ACCEL_CTRL_REG1, 0x00U);        /* standby */
    HAL_I2C_WriteReg(handle->address, ACCEL_XYZ_DATA_CFG_REG, 0x01U); /* +/-4g, 0.488 mg/LSB */
    HAL_I2C_WriteReg(handle->address, ACCEL_CTRL_REG1, 0x0DU);        /* 200 Hz active */
    return true;
}

/*
 * WHAT: Reads X/Y/Z and converts to milli-g, failing safely if the sensor
 * was never found or the read itself fails.
 * HOW: Zeros all three outputs FIRST, before any failure check, so every
 * exit path (NULL handle, not-found handle, failed I2C read) leaves the
 * caller with valid-looking (zeroed), not garbage, output values; same
 * unpacking/scaling math as every earlier accelerometer read in this
 * series.
 * WHY: This defensive-zeroing pattern matters more here than in the
 * earlier one-off demo files, because this driver is meant to be reused by
 * arbitrary future code that might not always check the boolean return
 * value carefully; failing safe here means a bug elsewhere reads plausible
 * zeros instead of uninitialized stack memory.
 */
bool Accel_ReadXYZ_mg(const accel_handle_t *handle, int16_t *xMg, int16_t *yMg, int16_t *zMg)
{
    uint8_t raw[6];
    int16_t xRaw, yRaw, zRaw;

    *xMg = 0;
    *yMg = 0;
    *zMg = 0;

    if ((handle == NULL) || !handle->found)
    {
        return false;
    }
    if (HAL_I2C_ReadRegs(handle->address, ACCEL_OUT_X_MSB_REG, raw, 6U) != kHalI2cOk)
    {
        return false;
    }

    /* Each axis is 16 bits read but only the top 14 are real data
       (left-justified); dividing by 4 drops the two padding bits. */
    xRaw = (int16_t)((uint16_t)(raw[0] << 8) | raw[1]) / 4;
    yRaw = (int16_t)((uint16_t)(raw[2] << 8) | raw[3]) / 4;
    zRaw = (int16_t)((uint16_t)(raw[4] << 8) | raw[5]) / 4;

    *xMg = (int16_t)((int32_t)xRaw * 488 / 1000);
    *yMg = (int16_t)((int32_t)yRaw * 488 / 1000);
    *zMg = (int16_t)((int32_t)zRaw * 488 / 1000);
    return true;
}
