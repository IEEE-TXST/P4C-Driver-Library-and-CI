/*
 * Unity tests for drivers/touch_tsi.c, running entirely on the host.
 * tests/mocks/mock_hal_tsi.c stands in for the TSI hardware.
 */
#include "unity.h"
#include "touch_tsi.h"
#include "mock_hal_tsi.h"

#define TEST_CHANNEL 9U /* TSI0_CH9, the same electrode P1/P2 used */

void setUp(void)
{
    MockHalTsi_Reset();
}

void tearDown(void)
{
}

void test_Init_StoresTheCalibratedBaseline(void)
{
    touch_handle_t handle;

    MockHalTsi_SetCalibrationResult(TEST_CHANNEL, 1500U);
    Touch_Init(&handle, TEST_CHANNEL);

    TEST_ASSERT_EQUAL_UINT32(TEST_CHANNEL, handle.channel);
    TEST_ASSERT_EQUAL_UINT16(1500U, handle.baseline);
}

void test_IsPressed_ReturnsFalse_WhenCounterIsAtBaseline(void)
{
    touch_handle_t handle;

    MockHalTsi_SetCalibrationResult(TEST_CHANNEL, 1500U);
    Touch_Init(&handle, TEST_CHANNEL);

    MockHalTsi_SetRawCounter(TEST_CHANNEL, 1500U); /* no change: nothing touching it */
    TEST_ASSERT_FALSE(Touch_IsPressed(&handle));
}

void test_IsPressed_ReturnsFalse_WhenCounterIsWithinNoiseFloor(void)
{
    touch_handle_t handle;

    MockHalTsi_SetCalibrationResult(TEST_CHANNEL, 1500U);
    Touch_Init(&handle, TEST_CHANNEL);

    MockHalTsi_SetRawCounter(TEST_CHANNEL, 1580U); /* +80, below the 100-count threshold */
    TEST_ASSERT_FALSE(Touch_IsPressed(&handle));
}

void test_IsPressed_ReturnsTrue_WhenCounterExceedsThreshold(void)
{
    touch_handle_t handle;

    MockHalTsi_SetCalibrationResult(TEST_CHANNEL, 1500U);
    Touch_Init(&handle, TEST_CHANNEL);

    MockHalTsi_SetRawCounter(TEST_CHANNEL, 1650U); /* +150, above the 100-count threshold */
    TEST_ASSERT_TRUE(Touch_IsPressed(&handle));
}

void test_IsPressed_ReturnsFalse_WhenCounterDropsBelowBaseline(void)
{
    touch_handle_t handle;

    /* A counter reading below baseline (drift, temperature, etc.) must
       never be misread as a touch; this is exactly the kind of edge case
       hardware-in-the-loop testing tends to miss because it's hard to
       reproduce on demand, but a mock reproduces it trivially. */
    MockHalTsi_SetCalibrationResult(TEST_CHANNEL, 1500U);
    Touch_Init(&handle, TEST_CHANNEL);

    MockHalTsi_SetRawCounter(TEST_CHANNEL, 1400U);
    TEST_ASSERT_FALSE(Touch_IsPressed(&handle));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Init_StoresTheCalibratedBaseline);
    RUN_TEST(test_IsPressed_ReturnsFalse_WhenCounterIsAtBaseline);
    RUN_TEST(test_IsPressed_ReturnsFalse_WhenCounterIsWithinNoiseFloor);
    RUN_TEST(test_IsPressed_ReturnsTrue_WhenCounterExceedsThreshold);
    RUN_TEST(test_IsPressed_ReturnsFalse_WhenCounterDropsBelowBaseline);
    return UNITY_END();
}
