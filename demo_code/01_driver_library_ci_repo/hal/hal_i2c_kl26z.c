/*
 * Real HAL_I2C implementation for the FRDM-KL26Z. Wraps the SDK's I2C0
 * driver exactly the way P1's verified accelerometer code did (same
 * blocking transfer pattern, same subaddress usage); only compiled into
 * app/, the real ARM firmware, never into the host-native test builds.
 */
#include <stdbool.h>
#include "hal_i2c.h"
#include "fsl_i2c.h"
#include "fsl_clock.h"

static bool s_initialized = false;
static i2c_master_transfer_t s_xfer;

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
