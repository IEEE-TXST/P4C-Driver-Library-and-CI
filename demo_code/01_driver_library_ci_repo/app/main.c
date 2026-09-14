/*
 * P4-C real firmware. Deliberately thin: everything that matters is in
 * drivers/accel_fxos8700.c and drivers/touch_tsi.c, the exact same source
 * files the host-native test suite in tests/ compiles and tests. This
 * file only wires them to the real hardware (via hal/hal_i2c_kl26z.c and
 * hal/hal_tsi_kl26z.c) and prints results over UART. If this builds and
 * runs correctly, it's direct proof the portable driver code, tested
 * without any board at all, also works on real hardware.
 */

/*
 * WHAT: Initializes the accelerometer and touch driver, then loops
 * forever printing both readings.
 * HOW: Calls only the portable driver API (Accel_Init, Touch_Init,
 * Accel_ReadXYZ_mg, Touch_IsPressed); which HAL implementation those calls
 * actually reach (hal_i2c_kl26z.c / hal_tsi_kl26z.c, the real hardware
 * versions) is decided entirely by which files this app/ target links
 * against, not by anything visible in this file.
 * WHY: Keeping this file this thin is the point of the whole P4-C
 * architecture: there's deliberately nothing here worth unit-testing on
 * its own (no register math, no parsing, no threshold logic), because all
 * of that already lives in drivers/, where it's already covered by
 * tests/test_accel_fxos8700.c and tests/test_touch_tsi.c running against
 * the mock HAL. This file's only real job is proving those same driver
 * files also compile and run correctly against real silicon.
 */
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "accel_fxos8700.h"
#include "touch_tsi.h"

#define TOUCH_CHANNEL 9U /* TSI0_CH9 */

/* Plain busy-wait, same shape as every earlier project's software delay;
   paces the print loop so output is readable, not a real-time requirement. */
static void Delay(void)
{
    volatile uint32_t i;
    for (i = 0U; i < 3000000U; i++)
    {
    }
}

int main(void)
{
    accel_handle_t accel;
    touch_handle_t touch;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\n=== P4-C driver library reference (app/) ===\r\n");

    /*
     * WHAT: One-time setup: find/configure the accelerometer, calibrate
     * the touch channel.
     * WHY: Accel_Init's success is checked and reported, but NOT treated
     * as fatal (no infinite-loop-and-hang if it fails), the same
     * graceful-degradation choice P1's dashboard made: a missing sensor
     * shouldn't prevent the rest of the system (touch sensing) from
     * working.
     */
    if (Accel_Init(&accel))
    {
        PRINTF("Accelerometer found at I2C address 0x%02X\r\n", accel.address);
    }
    else
    {
        PRINTF("WARNING: accelerometer not found.\r\n");
    }

    Touch_Init(&touch, TOUCH_CHANNEL);
    PRINTF("Touch channel %u calibrated, baseline = %u\r\n\r\n", TOUCH_CHANNEL, touch.baseline);

    /*
     * WHAT: Reads both sensors every pass and prints the result, forever.
     * WHY: Accel_ReadXYZ_mg's return value is checked every single time
     * (not just at init), since the driver is explicitly designed to be
     * able to fail on any individual read, not only at startup; this loop
     * demonstrates handling that per-read possibility rather than assuming
     * a successful Accel_Init guarantees every future read also succeeds.
     */
    for (;;)
    {
        int16_t x, y, z;
        bool pressed = Touch_IsPressed(&touch);

        if (Accel_ReadXYZ_mg(&accel, &x, &y, &z))
        {
            PRINTF("Accel X=%5d Y=%5d Z=%5d mg | touch=%s\r\n", x, y, z, pressed ? "yes" : "no ");
        }
        else
        {
            PRINTF("Accel read failed | touch=%s\r\n", pressed ? "yes" : "no ");
        }

        Delay();
    }
}
