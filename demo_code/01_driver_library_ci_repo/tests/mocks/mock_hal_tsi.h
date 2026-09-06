#ifndef MOCK_HAL_TSI_H
#define MOCK_HAL_TSI_H

#include <stdint.h>

void MockHalTsi_Reset(void);

/* HAL_TSI_Calibrate() for this channel returns this value. */
void MockHalTsi_SetCalibrationResult(uint32_t channel, uint16_t baseline);

/* HAL_TSI_ReadRawCounter() for this channel returns this value, every
   time it's called, until changed again. */
void MockHalTsi_SetRawCounter(uint32_t channel, uint16_t counter);

#endif /* MOCK_HAL_TSI_H */
