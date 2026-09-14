/*
 * WHAT: The touch-button logic: calibrate once, then compare each new
 * reading against that baseline.
 * HOW: Both functions are thin wrappers around HAL_TSI_Calibrate/
 * HAL_TSI_ReadRawCounter; all the actual hardware access happens in
 * whichever HAL implementation is linked in, never here.
 * WHY: This driver is deliberately this short: the "interesting" logic
 * (bit of a driver) is just one comparison, delta > TOUCH_THRESHOLD_COUNTS,
 * which is exactly the kind of small, pure logic tests/test_touch_tsi.c
 * can verify precisely by controlling what the mock HAL returns.
 */
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
