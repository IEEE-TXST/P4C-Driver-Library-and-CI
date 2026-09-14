/*
 * Mock HAL_I2C: satisfies the exact same hal_i2c.h prototypes
 * hal_i2c_kl26z.c does, but with plain arrays standing in for the I2C
 * bus and the sensor's registers, no hardware, no SDK headers, nothing
 * ARM-specific. This is the entire "mock hardware registers" concept in
 * one file: a driver written against hal_i2c.h cannot tell the
 * difference between this and the real thing.
 */

/*
 * WHAT: A fake I2C bus, implemented as plain in-memory tables, that a test
 * can program before calling driver code and inspect afterward.
 * HOW: Two small fixed-size tables do all the work: s_responses (what
 * HAL_I2C_ReadRegs should return for a given address+register, set up by
 * MockHalI2c_SetReadResponse) and s_writes (a log of every
 * HAL_I2C_WriteReg call, read back by MockHalI2c_GetWrite/GetWriteCount).
 * WHY: This file exists specifically so accel_fxos8700.c (or any future
 * driver written against hal_i2c.h) can be unit-tested without any real
 * I2C hardware present at all: a test "wires up" this fake bus to behave
 * like a specific real device (or a broken one, via ForceReadStatus),
 * calls the driver, then checks the driver's behavior and what it wrote,
 * all running as an ordinary host-native program.
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

/* Clears every programmed response and recorded write; every test calls
   this first so tests can never leak state into each other. */
void MockHalI2c_Reset(void)
{
    s_responseCount = 0U;
    s_writeCount = 0U;
    s_forceStatus = false;
    s_forcedStatus = kHalI2cOk;
    memset(s_responses, 0, sizeof(s_responses));
    memset(s_writes, 0, sizeof(s_writes));
}

/*
 * WHAT: Programs what HAL_I2C_ReadRegs should return for one specific
 * (deviceAddr, reg) pair.
 * HOW: If a response for the same address+register was already
 * programmed, overwrites it in place instead of adding a duplicate entry;
 * otherwise appends a new one, up to MAX_RESPONSES.
 * WHY: The overwrite check is what lets a test call this function multiple
 * times for the same register (e.g. to change what WHO_AM_I returns
 * partway through a test) without silently accumulating stale, unused
 * entries.
 */
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

/*
 * WHAT: Makes every future read fail with a given status, regardless of
 * address/register.
 * WHY: This is what lets a test simulate "the accelerometer isn't wired
 * up" or "the bus timed out" and verify the driver handles that failure
 * correctly (e.g. Accel_Init returning false), a scenario that's easy to
 * script here but hard to reliably reproduce on real hardware on demand.
 */
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

/*
 * WHAT: The mock's implementation of the real HAL_I2C_ReadRegs signature.
 * HOW: If a status was force-set (ForceReadStatus), returns that
 * immediately; otherwise searches s_responses for a matching
 * address+register and copies its programmed bytes into the caller's
 * buffer, capped at whichever of the caller's requested length or the
 * programmed length is smaller.
 * WHY: An address/register with no programmed response returns
 * kHalI2cErrorNack, deliberately mirroring what a real, absent I2C device
 * would do (no ACK); this is what makes Accel_Init's address-probing loop
 * behave identically whether it's running against this mock or real
 * hardware.
 */
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

/*
 * WHAT: The mock's implementation of HAL_I2C_WriteReg: instead of writing
 * to real hardware, just logs the write into s_writes for later
 * inspection.
 * WHY: This log is what lets a test assert a driver sent the RIGHT
 * configuration sequence (e.g. standby, then range, then active mode, in
 * that exact order) by calling MockHalI2c_GetWrite afterward, something
 * that's very hard to verify against real hardware without an actual logic
 * analyzer on the I2C bus.
 */
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
