# P4-C: Setup and Walkthrough

*Part of the P4-C manual split. See `P4C_1_Start_Here.md` for the full file list and how to use this manual.*

---

## 4. Additional Toolchain for This Project

Everything from prior projects still applies for `app/`, the real firmware half. New for the test-and-CI half: a native C compiler already on your laptop (`gcc` or `clang`; macOS and Linux have one by default, Windows needs MinGW or WSL), and a GitHub account. The Unity test framework itself is vendored directly into the repository (Section 8 explains why), so no separate install is needed for it.

## 6. Designing the HAL Boundary

`hal/hal_i2c.h` is the entire contract:

```c
typedef enum
{
    kHalI2cOk = 0,
    kHalI2cErrorNack,
    kHalI2cErrorTimeout
} hal_i2c_status_t;

hal_i2c_status_t HAL_I2C_ReadRegs(uint8_t deviceAddr, uint8_t reg, uint8_t *data, uint32_t len);
hal_i2c_status_t HAL_I2C_WriteReg(uint8_t deviceAddr, uint8_t reg, uint8_t value);
```

Nothing in this header mentions `I2C0`, `fsl_i2c.h`, or anything else specific to the KL26Z. That's deliberate: this header is a promise about *behavior* ("you can read and write device registers over some I2C-like bus"), not about *how* that behavior is achieved. Two files satisfy this exact contract:

- `hal/hal_i2c_kl26z.c`: the real one. Wraps `I2C_MasterTransferBlocking()` exactly the way P1's verified accelerometer code did (same subaddress usage, same blocking pattern), and is only ever compiled into `app/`.
- `tests/mocks/mock_hal_i2c.c`: the fake one, covered in Section 8.

`hal/hal_tsi.h` follows the identical pattern for the touch sensor, with its own real (`hal_tsi_kl26z.c`) and mock (`tests/mocks/mock_hal_tsi.c`) implementations.

## 7. The Portable Drivers

`drivers/accel_fxos8700.c` is the same register sequence P1 verified (`WHO_AM_I` at `0x0D` returns `0xC7`, `XYZ_DATA_CFG` at `0x0E` set to `0x01` for the `±4g` range, `CTRL_REG1` at `0x2A` going standby-then-active, `OUT_X_MSB` at `0x01` for the 6-byte read), reorganized to call only `HAL_I2C_ReadRegs()` and `HAL_I2C_WriteReg()`:

```c
bool Accel_Init(accel_handle_t *handle)
{
    for (i = 0U; i < sizeof(kAccelAddresses); i++)
    {
        uint8_t whoAmI = 0U;
        if ((HAL_I2C_ReadRegs(kAccelAddresses[i], ACCEL_WHOAMI_REG, &whoAmI, 1U) == kHalI2cOk) &&
            (whoAmI == ACCEL_WHOAMI_VALUE))
        {
            handle->address = kAccelAddresses[i];
            handle->found = true;
            break;
        }
    }
    /* ...configure via HAL_I2C_WriteReg()... */
}
```

Not one line of this file, or of `drivers/touch_tsi.c`, knows or cares whether it's about to run on a real KL26Z or a laptop with no board attached. That's not an accident of this particular driver; it's the entire architectural payoff of Section 6's boundary, made concrete.

## 8. Mocking the Hardware

`tests/mocks/mock_hal_i2c.c` satisfies `hal_i2c.h`'s exact function signatures using nothing but a couple of small fixed-size arrays:

```c
typedef struct
{
    uint8_t deviceAddr;
    uint8_t reg;
    uint8_t data[MAX_RESPONSE_BYTES];
    uint32_t len;
} programmed_response_t;

static programmed_response_t s_responses[MAX_RESPONSES];
```

A test calls `MockHalI2c_SetReadResponse(0x1DU, ACCEL_WHOAMI_REG, &whoAmI, 1U)` before running driver code, which stores "if anyone asks to read register `0x0D` from device `0x1D`, hand back this byte" in that array. When `HAL_I2C_ReadRegs()` is later called (indirectly, by the driver code under test), it just searches that array for a match and returns what it finds, or `kHalI2cErrorNack` if nothing was ever programmed for that address, exactly mimicking a real device that isn't there. `MockHalI2c_ForceReadStatus()` lets a test simulate any read failing outright, for testing error handling (Section 9) without needing a real, physically flaky I2C bus to reproduce a NACK on demand. This is the entire "mock hardware registers" concept: no `volatile`, no memory-mapped anything, ordinary C arrays a test can fully control.

**Unity itself is vendored, not installed.** `tests/unity/unity.c`, `unity.h`, and `unity_internals.h` are the real, unmodified upstream source (MIT licensed; `tests/unity/LICENSE.txt` is included), copied directly into the repository rather than fetched during the build. This means `make test` works immediately after a fresh `git clone`, with no separate install step, and CI needs no extra setup stage to go fetch a dependency before it can even start testing.

## 9. Writing the Tests

`tests/test_accel_fxos8700.c` covers both the success path and, deliberately, the failure paths, because a driver that's only ever tested with a cooperative, always-responding sensor hasn't really been tested:

```c
void test_ReadXYZ_ConvertsRawBytesToCorrectMg(void)
{
    uint8_t xyzBytes[6] = {0x10U, 0x00U, 0xF0U, 0x00U, 0x10U, 0x00U};
    /* ...init the mock sensor at 0x1D... */
    MockHalI2c_SetReadResponse(0x1DU, ACCEL_OUT_X_MSB_REG, xyzBytes, 6U);
    TEST_ASSERT_TRUE(Accel_ReadXYZ_mg(&handle, &x, &y, &z));
    TEST_ASSERT_EQUAL_INT16(499, x);
    TEST_ASSERT_EQUAL_INT16(-499, y);
    TEST_ASSERT_EQUAL_INT16(499, z);
}

void test_ReadXYZ_ReturnsFalseAndZeroes_WhenI2cReadFails(void)
{
    /* ...init succeeds, then: */
    MockHalI2c_ForceReadStatus(kHalI2cErrorNack);
    TEST_ASSERT_FALSE(Accel_ReadXYZ_mg(&handle, &x, &y, &z));
    TEST_ASSERT_EQUAL_INT16(0, x); /* and y, and z */
}
```

**Every expected numeric value in this test file was verified, not eyeballed.** The `499`/`-499` conversion result was computed by a small, standalone native C program using the exact same integer arithmetic as `accel_fxos8700.c` (right down to matching C's truncating integer division, which differs from some other languages' behavior on negative numbers, a real, easy-to-get-wrong detail this project's own verification process caught), before being hardcoded into the test as the known-correct answer.

`tests/test_touch_tsi.c` follows the same pattern for the touch driver, including a test that a counter reading *below* baseline (drift, temperature, anything) never gets misread as a touch, exactly the kind of edge case that's hard to reproduce on demand with real hardware but trivial to construct with a mock.

Both test files end with a plain `main()` calling `UNITY_BEGIN()`, a `RUN_TEST()` per test function, and `UNITY_END()`, no code-generation tooling needed:

```c
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Init_FindsDeviceAndRunsCorrectConfigSequence_WhenWhoAmIMatches);
    /* ... */
    return UNITY_END();
}
```

## 10. The Host-Native Makefile

```makefile
CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -Werror -Itests/unity -Ihal -Idrivers -Itests/mocks

test: build/test_accel_fxos8700 build/test_touch_tsi
	build/test_accel_fxos8700
	build/test_touch_tsi
```

Notice what's absent: no `arm-none-eabi-gcc`, no SDK, no board. This builds two ordinary native executables that link the driver source against the mocks, and just runs them; `-Werror` means even a compiler warning fails the build, not only an actual test assertion. **This was actually run, not just written to look plausible.** All 11 tests pass, `6 Tests 0 Failures 0 Ignored` for the accelerometer suite, `5 Tests 0 Failures 0 Ignored` for touch. A single expected value was then deliberately changed to a wrong one, and `make test` correctly failed (`FAIL: Expected 500 Was 499`) with a non-zero exit code, exactly what would turn a GitHub Actions check red, before being reverted; this confirms the suite genuinely detects a wrong answer, not just that it compiles.

## 11. Session 1 Deliverable: One Passing Test

The session's actual deliverable is small on purpose: `drivers/`, `hal/`, `tests/`, `app/` created in a real GitHub repository, `hal/hal_i2c.h` defined (Section 6), and one Unity test compiling and passing on the host via the Makefile (Section 9's simplest case is a good first target, confirming a device is found at the right address). Everything else, the second driver, the full error-case coverage, CI itself, is the independent work before Nov 19.

## 12. GitHub Actions CI

```yaml
name: CI
on:
  push:
  pull_request:
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build and run the driver test suite
        run: make test
```

This is deliberately the simplest workflow that does the real job: on every push and every pull request, GitHub spins up a fresh Ubuntu machine, checks out the repository, and runs exactly `make test`, the same command from Section 10. If that command's exit code is non-zero, GitHub marks the check as failed, visible directly on the commit and on any open pull request, without anyone needing to have run the tests locally first. This file has not been run on GitHub's actual servers as part of this manual's verification (that requires a real, pushed repository); what's been verified is that the command it runs, `make test`, genuinely works and genuinely detects failures (Section 10).

## 13. Pull Request Workflow

The practical Git workflow this project asks for: each group member branches off `main` for their own change, pushes it, and opens a pull request rather than pushing straight to `main`. GitHub Actions runs automatically on that PR (Section 12); a reviewer, another member of the group, checks that the CI check is green before approving and merging, not just that the code looks reasonable on read-through. This is a small-scale version of exactly how professional teams gate code review on automated checks, and it's worth actually practicing the mechanics (branch, push, open PR, wait for the check, review, merge) rather than treating it as a formality, since the muscle memory is the transferable skill here, not the specific repository.

## 14. Building and Bench-Testing the Real Firmware

`app/main.c` is deliberately thin:

```c
if (Accel_Init(&accel)) { PRINTF("Accelerometer found at I2C address 0x%02X\r\n", accel.address); }
Touch_Init(&touch, TOUCH_CHANNEL);
for (;;)
{
    bool pressed = Touch_IsPressed(&touch);
    if (Accel_ReadXYZ_mg(&accel, &x, &y, &z)) { PRINTF("Accel X=%5d Y=%5d Z=%5d mg | touch=%s\r\n", x, y, z, pressed ? "yes" : "no "); }
    Delay();
}
```

It links `drivers/accel_fxos8700.c` and `drivers/touch_tsi.c`, the *exact same files* Section 9's tests exercise, now compiled against `hal/hal_i2c_kl26z.c` and `hal/hal_tsi_kl26z.c` (the real implementations) instead of the mocks. If this builds, flashes, and prints sane readings on a real board, that's direct, concrete evidence that testing without hardware (Section 9) produced code that actually works on hardware, not just code that satisfies a mock's expectations. Build it the same way as every prior project's demo code, `cmake` / `make` / `objcopy`, covered in the P0 manual, Section 8.

---

**Next:** `P4C_4_Reference.md` for code structure, sample output, debugging, and the glossary.

---
*IEEE Texas State University Student Branch. Connect. Build. Inspire.*
