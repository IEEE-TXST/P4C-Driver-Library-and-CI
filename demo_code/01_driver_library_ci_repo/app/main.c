/*
 * P4-C real firmware. Deliberately thin: everything that matters is in
 * drivers/accel_fxos8700.c and drivers/touch_tsi.c, the exact same source
 * files the host-native test suite in tests/ compiles and tests. This
 * file only wires them to the real hardware (via hal/hal_i2c_kl26z.c and
 * hal/hal_tsi_kl26z.c) and prints results over UART. If this builds and
 * runs correctly, it's direct proof the portable driver code, tested
 * without any board at all, also works on real hardware.
 */
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "accel_fxos8700.h"
#include "touch_tsi.h"

#define TOUCH_CHANNEL 9U /* TSI0_CH9 */

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
