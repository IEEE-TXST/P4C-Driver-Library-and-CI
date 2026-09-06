/*
 * Unity tests for drivers/accel_fxos8700.c, running entirely on the host,
 * no board, no I2C0, no SDK. tests/mocks/mock_hal_i2c.c stands in for the
 * hardware; every expected value here was cross-checked against a small
 * native C program before being hardcoded (see the manual, Section 9),
 * not eyeballed.
 */
#include "unity.h"
#include "accel_fxos8700.h"
#include "mock_hal_i2c.h"

#define ACCEL_WHOAMI_REG 0x0DU
#define ACCEL_XYZ_DATA_CFG_REG 0x0EU
#define ACCEL_CTRL_REG1 0x2AU
#define ACCEL_OUT_X_MSB_REG 0x01U
#define ACCEL_WHOAMI_VALUE 0xC7U

void setUp(void)
{
    MockHalI2c_Reset();
}

void tearDown(void)
{
}

void test_Init_FindsDeviceAndRunsCorrectConfigSequence_WhenWhoAmIMatches(void)
{
    uint8_t whoAmI = ACCEL_WHOAMI_VALUE;
    accel_handle_t handle;
    uint8_t addr, reg, value;

    /* Only address 0x1D answers; 0x1C, 0x1E, 0x1F are left unprogrammed,
       which the mock treats as "nothing there," exactly like a real,
       silent I2C address. */
    MockHalI2c_SetReadResponse(0x1DU, ACCEL_WHOAMI_REG, &whoAmI, 1U);

    TEST_ASSERT_TRUE(Accel_Init(&handle));
    TEST_ASSERT_TRUE(handle.found);
    TEST_ASSERT_EQUAL_HEX8(0x1DU, handle.address);

    /* Confirm the exact 3-write configuration sequence P1 verified:
       standby, then range select, then active mode. */
    TEST_ASSERT_EQUAL_UINT32(3U, MockHalI2c_GetWriteCount());

    MockHalI2c_GetWrite(0U, &addr, &reg, &value);
    TEST_ASSERT_EQUAL_HEX8(0x1DU, addr);
    TEST_ASSERT_EQUAL_HEX8(ACCEL_CTRL_REG1, reg);
    TEST_ASSERT_EQUAL_HEX8(0x00U, value);

    MockHalI2c_GetWrite(1U, &addr, &reg, &value);
    TEST_ASSERT_EQUAL_HEX8(ACCEL_XYZ_DATA_CFG_REG, reg);
    TEST_ASSERT_EQUAL_HEX8(0x01U, value);

    MockHalI2c_GetWrite(2U, &addr, &reg, &value);
    TEST_ASSERT_EQUAL_HEX8(ACCEL_CTRL_REG1, reg);
    TEST_ASSERT_EQUAL_HEX8(0x0DU, value);
}

void test_Init_ReturnsFalse_WhenNoDeviceRespondsAtAnyAddress(void)
{
    accel_handle_t handle;

    /* No responses programmed at all: every address NACKs. */
    TEST_ASSERT_FALSE(Accel_Init(&handle));
    TEST_ASSERT_FALSE(handle.found);
    TEST_ASSERT_EQUAL_UINT32(0U, MockHalI2c_GetWriteCount()); /* never got far enough to configure it */
}

void test_Init_ReturnsFalse_WhenDeviceRespondsButWhoAmIIsWrong(void)
{
    uint8_t wrongId = 0x1AU; /* a real value, but the wrong sensor (MMA8451, not FXOS8700) */
    accel_handle_t handle;

    MockHalI2c_SetReadResponse(0x1DU, ACCEL_WHOAMI_REG, &wrongId, 1U);

    TEST_ASSERT_FALSE(Accel_Init(&handle));
    TEST_ASSERT_FALSE(handle.found);
}

void test_ReadXYZ_ConvertsRawBytesToCorrectMg(void)
{
    uint8_t whoAmI = ACCEL_WHOAMI_VALUE;
    /* X = 0x1000 (14-bit 1024, +499 mg), Y = 0xF000 (14-bit -1024, -499 mg),
       Z = 0x1000 again. Cross-checked with a native C program using the
       exact same integer math as accel_fxos8700.c; see the manual. */
    uint8_t xyzBytes[6] = {0x10U, 0x00U, 0xF0U, 0x00U, 0x10U, 0x00U};
    accel_handle_t handle;
    int16_t x, y, z;

    MockHalI2c_SetReadResponse(0x1DU, ACCEL_WHOAMI_REG, &whoAmI, 1U);
    TEST_ASSERT_TRUE(Accel_Init(&handle));

    MockHalI2c_SetReadResponse(0x1DU, ACCEL_OUT_X_MSB_REG, xyzBytes, 6U);
    TEST_ASSERT_TRUE(Accel_ReadXYZ_mg(&handle, &x, &y, &z));

    TEST_ASSERT_EQUAL_INT16(499, x);
    TEST_ASSERT_EQUAL_INT16(-499, y);
    TEST_ASSERT_EQUAL_INT16(499, z);
}

void test_ReadXYZ_ReturnsFalseAndZeroes_WhenI2cReadFails(void)
{
    uint8_t whoAmI = ACCEL_WHOAMI_VALUE;
    accel_handle_t handle;
    int16_t x = 111, y = 222, z = 333; /* deliberately non-zero, to prove they get reset */

    MockHalI2c_SetReadResponse(0x1DU, ACCEL_WHOAMI_REG, &whoAmI, 1U);
    TEST_ASSERT_TRUE(Accel_Init(&handle));

    MockHalI2c_ForceReadStatus(kHalI2cErrorNack);
    TEST_ASSERT_FALSE(Accel_ReadXYZ_mg(&handle, &x, &y, &z));

    TEST_ASSERT_EQUAL_INT16(0, x);
    TEST_ASSERT_EQUAL_INT16(0, y);
    TEST_ASSERT_EQUAL_INT16(0, z);
}

void test_ReadXYZ_ReturnsFalse_WhenNeverInitialized(void)
{
    accel_handle_t handle = {0U, false}; /* as if Accel_Init() was never called, or failed */
    int16_t x, y, z;

    TEST_ASSERT_FALSE(Accel_ReadXYZ_mg(&handle, &x, &y, &z));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Init_FindsDeviceAndRunsCorrectConfigSequence_WhenWhoAmIMatches);
    RUN_TEST(test_Init_ReturnsFalse_WhenNoDeviceRespondsAtAnyAddress);
    RUN_TEST(test_Init_ReturnsFalse_WhenDeviceRespondsButWhoAmIIsWrong);
    RUN_TEST(test_ReadXYZ_ConvertsRawBytesToCorrectMg);
    RUN_TEST(test_ReadXYZ_ReturnsFalseAndZeroes_WhenI2cReadFails);
    RUN_TEST(test_ReadXYZ_ReturnsFalse_WhenNeverInitialized);
    return UNITY_END();
}
