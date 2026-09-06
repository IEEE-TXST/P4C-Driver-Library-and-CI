#include "touch_tsi.h"
#include "hal_tsi.h"

#define TOUCH_THRESHOLD_COUNTS 100

void Touch_Init(touch_handle_t *handle, uint32_t channel)
{
    handle->channel = channel;
    handle->baseline = HAL_TSI_Calibrate(channel);
}

bool Touch_IsPressed(const touch_handle_t *handle)
{
    uint16_t counter = HAL_TSI_ReadRawCounter(handle->channel);
    int32_t delta = (int32_t)counter - (int32_t)handle->baseline;
    return delta > TOUCH_THRESHOLD_COUNTS;
}
