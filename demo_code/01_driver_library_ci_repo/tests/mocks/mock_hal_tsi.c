/*
 * Mock HAL_TSI: same idea as mock_hal_i2c.c, plain fixed-size arrays
 * (indexed by channel number) standing in for TSI0's calibration state
 * and its live counter register.
 */

/*
 * WHAT: A fake TSI peripheral, implemented as two plain arrays indexed by
 * channel number, so touch_tsi.c can be unit-tested without any touch
 * hardware present.
 * HOW: s_calibration holds each channel's programmed baseline; s_rawCounter
 * holds each channel's programmed "current reading"; the hal_tsi.h
 * functions below just return whichever array entry matches the requested
 * channel.
 * WHY: Simpler than mock_hal_i2c.c because TSI's interface is simpler
 * (no address/register pairs, no write log needed): a test just sets a
 * channel's calibration and counter directly, calls the driver, and
 * checks the boolean result Touch_IsPressed returns.
 */
#include "mock_hal_tsi.h"

#define MAX_CHANNELS 16U

static uint16_t s_calibration[MAX_CHANNELS];
static uint16_t s_rawCounter[MAX_CHANNELS];

void MockHalTsi_Reset(void)
{
    uint32_t i;
    for (i = 0U; i < MAX_CHANNELS; i++)
    {
        s_calibration[i] = 0U;
        s_rawCounter[i] = 0U;
    }
}

void MockHalTsi_SetCalibrationResult(uint32_t channel, uint16_t baseline)
{
    if (channel < MAX_CHANNELS)
    {
        s_calibration[channel] = baseline;
    }
}

void MockHalTsi_SetRawCounter(uint32_t channel, uint16_t counter)
{
    if (channel < MAX_CHANNELS)
    {
        s_rawCounter[channel] = counter;
    }
}

/* ---------------- hal_tsi.h implementation ---------------- */

void HAL_TSI_Init(void)
{
    /* Nothing to do for the mock; no hardware module to enable. */
}

/*
 * WHAT: Returns whatever baseline was programmed for this channel via
 * MockHalTsi_SetCalibrationResult, instead of running a real calibration
 * sweep.
 * WHY: Bounds-checking `channel < MAX_CHANNELS` before indexing (here and
 * in ReadRawCounter below) is what keeps an out-of-range channel number
 * from reading past the end of these fixed-size arrays; returning 0U for
 * an invalid channel is a safe, deterministic fallback for a test double.
 */
uint16_t HAL_TSI_Calibrate(uint32_t channel)
{
    if (channel < MAX_CHANNELS)
    {
        return s_calibration[channel];
    }
    return 0U;
}

uint16_t HAL_TSI_ReadRawCounter(uint32_t channel)
{
    if (channel < MAX_CHANNELS)
    {
        return s_rawCounter[channel];
    }
    return 0U;
}
