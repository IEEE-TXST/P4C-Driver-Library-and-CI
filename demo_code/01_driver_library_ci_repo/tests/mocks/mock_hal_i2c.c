/*
 * Mock HAL_I2C: satisfies the exact same hal_i2c.h prototypes
 * hal_i2c_kl26z.c does, but with plain arrays standing in for the I2C
 * bus and the sensor's registers, no hardware, no SDK headers, nothing
 * ARM-specific. This is the entire "mock hardware registers" concept in
 * one file: a driver written against hal_i2c.h cannot tell the
 * difference between this and the real thing.
 */
#include <string.h>
#include <stdbool.h>
#include "mock_hal_i2c.h"

#define MAX_RESPONSES 8U
#define MAX_WRITES 16U
#define MAX_RESPONSE_BYTES 8U

typedef struct
{
    uint8_t deviceAddr;
    uint8_t reg;
    uint8_t data[MAX_RESPONSE_BYTES];
    uint32_t len;
} programmed_response_t;

typedef struct
{
    uint8_t deviceAddr;
    uint8_t reg;
    uint8_t value;
} write_record_t;

static programmed_response_t s_responses[MAX_RESPONSES];
static uint32_t s_responseCount = 0U;

static write_record_t s_writes[MAX_WRITES];
static uint32_t s_writeCount = 0U;

static bool s_forceStatus = false;
static hal_i2c_status_t s_forcedStatus = kHalI2cOk;

void MockHalI2c_Reset(void)
{
    s_responseCount = 0U;
    s_writeCount = 0U;
    s_forceStatus = false;
    s_forcedStatus = kHalI2cOk;
    memset(s_responses, 0, sizeof(s_responses));
    memset(s_writes, 0, sizeof(s_writes));
}

void MockHalI2c_SetReadResponse(uint8_t deviceAddr, uint8_t reg, const uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint32_t copyLen = (len > MAX_RESPONSE_BYTES) ? MAX_RESPONSE_BYTES : len;

    /* Overwrite an existing programmed response for the same address/register, if any. */
    for (i = 0U; i < s_responseCount; i++)
    {
        if ((s_responses[i].deviceAddr == deviceAddr) && (s_responses[i].reg == reg))
        {
            memcpy(s_responses[i].data, data, copyLen);
            s_responses[i].len = copyLen;
            return;
        }
    }

    if (s_responseCount < MAX_RESPONSES)
    {
        s_responses[s_responseCount].deviceAddr = deviceAddr;
        s_responses[s_responseCount].reg = reg;
        memcpy(s_responses[s_responseCount].data, data, copyLen);
        s_responses[s_responseCount].len = copyLen;
        s_responseCount++;
    }
}

void MockHalI2c_ForceReadStatus(hal_i2c_status_t status)
{
    s_forceStatus = true;
    s_forcedStatus = status;
}

uint32_t MockHalI2c_GetWriteCount(void)
{
    return s_writeCount;
}

void MockHalI2c_GetWrite(uint32_t index, uint8_t *deviceAddr, uint8_t *reg, uint8_t *value)
{
    if (index < s_writeCount)
    {
        *deviceAddr = s_writes[index].deviceAddr;
        *reg = s_writes[index].reg;
        *value = s_writes[index].value;
    }
}

/* ---------------- hal_i2c.h implementation, backed entirely by the
   plain arrays above instead of any real register. ---------------- */

hal_i2c_status_t HAL_I2C_ReadRegs(uint8_t deviceAddr, uint8_t reg, uint8_t *data, uint32_t len)
{
    uint32_t i;

    if (s_forceStatus)
    {
        return s_forcedStatus;
    }

    for (i = 0U; i < s_responseCount; i++)
    {
        if ((s_responses[i].deviceAddr == deviceAddr) && (s_responses[i].reg == reg))
        {
            uint32_t copyLen = (len < s_responses[i].len) ? len : s_responses[i].len;
            memcpy(data, s_responses[i].data, copyLen);
            return kHalI2cOk;
        }
    }

    /* No test ever programmed a response for this address/register: same
       as a real device that isn't there. */
    return kHalI2cErrorNack;
}

hal_i2c_status_t HAL_I2C_WriteReg(uint8_t deviceAddr, uint8_t reg, uint8_t value)
{
    if (s_writeCount < MAX_WRITES)
    {
        s_writes[s_writeCount].deviceAddr = deviceAddr;
        s_writes[s_writeCount].reg = reg;
        s_writes[s_writeCount].value = value;
        s_writeCount++;
    }

    if (s_forceStatus)
    {
        return s_forcedStatus;
    }
    return kHalI2cOk;
}
