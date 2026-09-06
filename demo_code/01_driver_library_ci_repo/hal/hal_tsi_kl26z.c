/*
 * Real HAL_TSI implementation for the FRDM-KL26Z. Wraps the SDK's TSI
 * driver exactly the way P1/P2's verified touch code did; only compiled
 * into app/, never into the host-native test builds.
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
