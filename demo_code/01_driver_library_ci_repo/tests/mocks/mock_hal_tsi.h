/*
 * WHAT: Test-side control surface for the fake TSI "hardware"
 * mock_hal_tsi.c implements: set what calibration and live-counter reads
 * return, per channel.
 * WHY: Same separation as mock_hal_i2c.h: touch_tsi.c only ever calls the
 * real hal_tsi.h functions; these Mock*-prefixed functions exist purely
 * for tests to set up and control that fake hardware.
 */
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
