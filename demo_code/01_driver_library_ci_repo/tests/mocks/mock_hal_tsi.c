/*
 * Mock HAL_TSI: same idea as mock_hal_i2c.c, plain fixed-size arrays
 * (indexed by channel number) standing in for TSI0's calibration state
 * and its live counter register.
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
