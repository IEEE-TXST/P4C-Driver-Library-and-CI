/*
 * HAL TSI port: same pattern as hal_i2c.h, a boundary drivers/touch_tsi.c
 * calls through instead of touching the SDK's TSI driver directly.
 *   - hal_tsi_kl26z.c: real implementation, wraps fsl_tsi_v4.h, ARM only.
 *   - tests/mocks/mock_hal_tsi.c: fake implementation, host only.
 */
#ifndef HAL_TSI_H
#define HAL_TSI_H

#include <stdint.h>

/* One-time module init (clock, module enable). Safe to call more than
   once; real hardware and the mock both treat repeated calls as a no-op
   after the first. */
void HAL_TSI_Init(void);

/* Runs the hardware's calibration routine for "channel" and returns the
   untouched baseline counter value. */
uint16_t HAL_TSI_Calibrate(uint32_t channel);

/* Reads one fresh raw counter value from "channel". */
uint16_t HAL_TSI_ReadRawCounter(uint32_t channel);

#endif /* HAL_TSI_H */
