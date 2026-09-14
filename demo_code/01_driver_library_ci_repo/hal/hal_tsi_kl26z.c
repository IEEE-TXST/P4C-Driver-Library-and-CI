/*
 * Real HAL_TSI implementation for the FRDM-KL26Z. Wraps the SDK's TSI
 * driver exactly the way P1/P2's verified touch code did; only compiled
 * into app/, never into the host-native test builds.
 */

/*
 * WHAT: The real-hardware side of the HAL_TSI port: every function here
 * actually drives this chip's TSI0 peripheral.
 * HOW: Same software-triggered polling read and calibration sequence used
 * throughout this series (P1's touch demo, P2's TouchSliderTask); the
 * lazy-init pattern (s_initialized) mirrors hal_i2c_kl26z.c exactly.
 * WHY: As with hal_i2c_kl26z.c, this file's whole reason to exist
 * separately from drivers/touch_tsi.c is so the SDK-specific TSI calls
 * (TSI_Init, TSI_StartSoftwareTrigger, etc.) never appear in the portable
 * driver code at all.
 */
#include <stdbool.h>
#include "hal_tsi.h"
#include "fsl_tsi_v4.h"

static bool s_initialized = false;

void HAL_TSI_Init(void)
{
    tsi_config_t tsiConfig;
    if (s_initialized)
    {
        return;
    }
    TSI_GetNormalModeDefaultConfig(&tsiConfig);
    TSI_Init(TSI0, &tsiConfig);
    TSI_EnableModule(TSI0, true);
    s_initialized = true;
}

/* Shared by both HAL_TSI_Calibrate and HAL_TSI_ReadRawCounter: selects a
   channel, triggers a software scan, and blocks until it completes. */
static uint16_t ReadOneCounter(uint32_t channel)
{
    uint16_t counter;
    TSI_SetMeasuredChannelNumber(TSI0, channel);
    TSI_StartSoftwareTrigger(TSI0);
    while (0U == (TSI_GetStatusFlags(TSI0) & kTSI_EndOfScanFlag))
    {
    }
    counter = TSI_GetCounter(TSI0);
    TSI_ClearStatusFlags(TSI0, kTSI_EndOfScanFlag);
    return counter;
}

/*
 * WHAT: Runs a full calibration pass and returns just the one requested
 * channel's baseline.
 * HOW: HAL_TSI_Init() is called here too (not just relying on the caller
 * having called it already), the same lazy-init safety net as
 * hal_i2c_kl26z.c's EnsureInitialized; TSI_Calibrate measures ALL channels
 * at once into a local struct, and only the requested channel's value is
 * returned.
 */
uint16_t HAL_TSI_Calibrate(uint32_t channel)
{
    tsi_calibration_data_t baseline;
    HAL_TSI_Init();
    TSI_Calibrate(TSI0, &baseline);
    return baseline.calibratedData[channel];
}

uint16_t HAL_TSI_ReadRawCounter(uint32_t channel)
{
    return ReadOneCounter(channel);
}
