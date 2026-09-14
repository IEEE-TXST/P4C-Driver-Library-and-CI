/*
 * Real HAL_I2C implementation for the FRDM-KL26Z. Wraps the SDK's I2C0
 * driver exactly the way P1's verified accelerometer code did (same
 * blocking transfer pattern, same subaddress usage); only compiled into
 * app/, the real ARM firmware, never into the host-native test builds.
 */

/*
 * WHAT: The "real hardware" side of the HAL_I2C port declared in
 * hal_i2c.h: every function here actually talks to this chip's I2C0
 * peripheral.
 * HOW: Same GetDefaultConfig/Init/blocking-transfer pattern used
 * throughout this whole series (P1's accelerometer demo, P4-A's command
 * handlers); the one addition here is EnsureInitialized, which lazily runs
 * I2C_MasterInit on the first call instead of requiring a separate
 * explicit init step.
 * WHY: This file is compiled ONLY into app/ (see the build setup this
 * project's manual describes), never into the host-native test binary;
 * mock_hal_i2c.c in tests/mocks/ is what stands in for this file there.
 * Nothing outside this file (and mock_hal_i2c.c) is allowed to know these
 * two implementations even exist, drivers/accel_fxos8700.c just calls
 * HAL_I2C_ReadRegs/HAL_I2C_WriteReg and gets whichever version the build
 * links in.
 */
#include <stdbool.h>
#include "hal_i2c.h"
#include "fsl_i2c.h"
#include "fsl_clock.h"

static bool s_initialized = false;
static i2c_master_transfer_t s_xfer;

/*
 * WHAT: Initializes I2C0 hardware the first time it's actually needed, and
 * does nothing on every subsequent call.
 * WHY: Lazy initialization (rather than requiring the application to
 * remember to call an explicit HAL_I2C_Init) means any driver code that
 * calls HAL_I2C_ReadRegs/WriteReg just works, without every caller needing
 * to coordinate who's responsible for the one-time setup.
 */
static void EnsureInitialized(void)
{
    i2c_master_config_t masterConfig;
    if (s_initialized)
    {
        return;
    }
    I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = 100000U;
    I2C_MasterInit(I2C0, &masterConfig, CLOCK_GetFreq(I2C0_CLK_SRC));
    s_initialized = true;
}

/*
 * WHAT / HOW: Builds an i2c_master_transfer_t for a multi-byte register
 * read and runs it via the SDK's blocking transfer call, translating the
 * SDK's own success/failure status into this HAL's hardware-agnostic
 * hal_i2c_status_t.
 * WHY: The translation at the end (kStatus_Success -> kHalI2cOk, anything
 * else -> kHalI2cErrorNack) is what keeps the SDK's own status codes from
 * leaking into drivers/accel_fxos8700.c, which only ever sees the plain
 * HAL enum defined in hal_i2c.h.
 */
hal_i2c_status_t HAL_I2C_ReadRegs(uint8_t deviceAddr, uint8_t reg, uint8_t *data, uint32_t len)
{
    EnsureInitialized();

    s_xfer.slaveAddress = deviceAddr;
    s_xfer.direction = kI2C_Read;
    s_xfer.subaddress = reg;
    s_xfer.subaddressSize = 1U;
    s_xfer.data = data;
    s_xfer.dataSize = len;
    s_xfer.flags = kI2C_TransferDefaultFlag;

    if (I2C_MasterTransferBlocking(I2C0, &s_xfer) != kStatus_Success)
    {
        return kHalI2cErrorNack;
    }
    return kHalI2cOk;
}

/* Same shape as HAL_I2C_ReadRegs, for a single-byte register write. */
hal_i2c_status_t HAL_I2C_WriteReg(uint8_t deviceAddr, uint8_t reg, uint8_t value)
{
    uint8_t buf[1];

    EnsureInitialized();

    buf[0] = value;
    s_xfer.slaveAddress = deviceAddr;
    s_xfer.direction = kI2C_Write;
    s_xfer.subaddress = reg;
    s_xfer.subaddressSize = 1U;
    s_xfer.data = buf;
    s_xfer.dataSize = 1U;
    s_xfer.flags = kI2C_TransferDefaultFlag;

    if (I2C_MasterTransferBlocking(I2C0, &s_xfer) != kStatus_Success)
    {
        return kHalI2cErrorNack;
    }
    return kHalI2cOk;
}
