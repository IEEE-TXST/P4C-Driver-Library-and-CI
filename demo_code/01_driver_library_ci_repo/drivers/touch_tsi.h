/*
 * Portable TSI touch-button driver: one electrode, thresholded against
 * its own calibrated baseline. Talks to the hardware only through
 * hal_tsi.h, same portability pattern as accel_fxos8700.c.
 */

/*
 * WHAT: Declares a minimal touch-button interface: initialize/calibrate
 * one channel, then ask whether it's currently pressed.
 * WHY: touch_handle_t holds only what's needed to answer that question
 * later (which channel, and its calibrated baseline), mirroring
 * accel_handle_t's minimal-state design in the accelerometer driver.
 */
#ifndef TOUCH_TSI_H
#define TOUCH_TSI_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t channel;
    uint16_t baseline;
} touch_handle_t;

/* Initializes the TSI module and calibrates "channel", storing its
   untouched baseline in "handle". */
void Touch_Init(touch_handle_t *handle, uint32_t channel);

/* Reads one fresh sample and returns true if it's far enough above the
   calibrated baseline to count as a touch. Same noise-floor threshold
   (100 counts) P1's touch slider work used. */
bool Touch_IsPressed(const touch_handle_t *handle);

#endif /* TOUCH_TSI_H */
