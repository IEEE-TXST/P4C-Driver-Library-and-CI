/*
 * Portable FXOS8700CQ driver. Register map and conversion verified in P1
 * (see P1_Project_Manual.md, Section 12): WHO_AM_I at 0x0D returns 0xC7,
 * XYZ_DATA_CFG at 0x0E set to 0x01 selects +/-4g range (0.488 mg/LSB),
 * CTRL_REG1 at 0x2A goes 0x00 (standby) then 0x0D (200 Hz active), and
 * OUT_X_MSB at 0x01 starts a 6-byte X/Y/Z read. Every register access
 * goes through hal_i2c.h, never touches I2C0 directly.
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
