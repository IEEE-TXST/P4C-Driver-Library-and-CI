# TXST IEEE Student Branch: FRDM-KL26Z Project Series
## Capstone Manual, P4-C: Embedded Driver Library and CI Pipeline

**Document status:** DRAFT v0.1, for project-leader review and bench testing before member use
**Track:** Embedded Systems and Software | **Difficulty:** Medium | **Sessions:** 1 (WS8, Nov 5, 2026), plus independent build before the Nov 19 demo
**Groups:** 2 groups of 3, working independently
**Companion documents:** P0's manual (`P0_Board_Orientation_and_Toolchain_Setup/`, start at `P0_1_Start_Here.md`) through `P3_Project_Manual.md` (read all four first), *TXST IEEE FRDM-KL26Z Project Specification*
**Hardware:** FRDM-KL26Z (only needed for the final `app/` bench test; most of this project runs on a laptop with no board attached)
**Prerequisite:** a GitHub account, free, for every member
**Demo code:** see `demo_code/` in this project's folder

---

## How to Use This Manual

Same rule as every prior manual: a reference, not required reading. Build the repo structure and get one test passing yourself first; open `demo_code/` only when stuck.

This project reads differently from every embedded project before it, on purpose. There is comparatively little hardware debugging here (Section 0 explains why), and comparatively more software architecture: how you draw the line between "code that touches a register" and "code that doesn't," and what that line buys you. Take the software engineering content as seriously as the embedded content; for a lot of hiring managers, this is the project on your resume that reads as most immediately recognizable.

Project leaders: bench-test `demo_code/01_driver_library_ci_repo/` before WS8, in both senses this project actually has: run `make test` on your own laptop and confirm all 11 tests pass, and separately, flash `app/` to a real board and confirm the printed accelerometer and touch readings look sane. Also worth doing once, if you have time: actually push the reference repo to a scratch GitHub repository and watch the Actions tab go green, so you've seen the real CI pipeline work, not just read a YAML file that looks correct.

**A note on accuracy:** this project's test suite was not just written, it was run, repeatedly, and a test was deliberately broken and confirmed to fail correctly before being fixed again, the strongest verification available short of literally watching GitHub Actions execute. The real embedded firmware was compiled and confirmed to link the *exact same* driver source files the test suite exercises. Section 21 has the full trail.

---

## 0. Why This Session Exists

Every project before this one asked "does the firmware work." This one asks a different, equally real question: "how do you know your firmware works, and how do you keep knowing that as more people touch the code?" A chapter with 25 boards and rotating membership is exactly the situation where untested, unreviewed embedded code quietly breaks in ways nobody notices until a demo fails. The pattern this project teaches, separate the hardware-touching code from the logic, mock the hardware for testing, automate the test run on every change, is not a student-project simplification of how real companies work; it is, close to verbatim, how real companies work. A firmware engineer who can say "I built a driver library with a mocked test suite and CI" in an interview is describing a genuinely professional practice, not a toy exercise.

## 1. New Concepts You Need Before Starting

Builds on P1 (the accelerometer and touch sensor this project's drivers wrap, reused without re-deriving) and general comfort with C from P2/P3. Everything below is new to P4-C.

**Hardware Abstraction Layer (HAL).** A thin layer of code whose entire job is hiding "how do I actually talk to this specific chip's registers" behind a small, stable set of function calls. Code above the HAL (a driver, an application) calls those functions and never touches a register directly; code below the HAL is the only place that does. The payoff: swap what's below the HAL (a different chip, or, as this project does, no chip at all) and everything above it keeps working unmodified.

**Mocking.** Replacing a real dependency, here, actual hardware, with a fake stand-in that behaves predictably and is fully controlled by the test writer. A mock doesn't try to simulate real I2C electrical behavior; it just remembers "if asked to read register X, return these bytes," using nothing more exotic than a plain array. This is the literal meaning of "replace volatile register pointers with plain variables in test builds": the mock's internal state is genuinely just ordinary C variables, no `volatile`, no memory-mapped anything.

**Unit test.** A small, automated check that one specific piece of code behaves correctly, given specific inputs, independent of everything else in the system. "The accelerometer driver converts these exact raw bytes to this exact milli-g value" is a unit test; "the whole board works" is not, that's an integration or system test, a different (and, for a microcontroller, much harder to automate) thing.

**Unity.** A small, dependency-free C testing framework, purpose-built for exactly this kind of embedded use case: no dynamic memory allocation required, no C++ needed, compiles and runs anywhere a C compiler exists, including directly on the chip if you wanted to (this project doesn't; the tests run on the host). `TEST_ASSERT_EQUAL_INT16(expected, actual)` and similar macros are Unity's entire vocabulary; Section 9 shows them in real use.

**Continuous Integration (CI).** Automatically running your test suite every time code changes, on a server, not manually on your own machine, so nobody can forget, and so a broken test is visible to the whole team, not just whoever happened to run it locally. GitHub Actions is one specific, free CI service tightly integrated with GitHub itself; Section 12 covers the exact workflow file this project uses.

**Repository structure as documentation.** `drivers/`, `hal/`, `tests/`, `app/` isn't an arbitrary folder scheme; each name is a promise about what kind of code lives there and what it's allowed to depend on. A new contributor (or your future self in six months) can navigate this repository correctly without reading a single line of code, purely from the folder names, which is exactly the point.

## 2. Purpose

By the end of this project, every group has: a GitHub repository laid out as `drivers/`, `hal/`, `tests/`, `app/`; a hardware abstraction layer with a real and a mocked implementation of the same interface; a Unity test suite covering both the accelerometer and touch drivers, including error cases; a GitHub Actions workflow running that suite on every push; and a README with a CI badge, build instructions, and driver API documentation, demonstrated at the Nov 19 showcase with the badge showing green.

## 3. Prerequisites

P1 (all peripherals), P2, and P3 all complete, plus a free GitHub account for every member of the group. This project reuses P1's exact, already-verified accelerometer register sequence and touch threshold logic; if that feels shaky, revisit `P1_Project_Manual.md` first.

## 4. Additional Toolchain for This Project

Everything from prior projects still applies for `app/`, the real firmware half. New for the test-and-CI half: a native C compiler already on your laptop (`gcc` or `clang`; macOS and Linux have one by default, Windows needs MinGW or WSL), and a GitHub account. The Unity test framework itself is vendored directly into the repository (Section 8 explains why), so no separate install is needed for it.

## 5. Repository Structure

```
drivers/    Portable sensor drivers. Call only hal/*.h functions, never a register.
hal/        The hardware boundary: hal_*_kl26z.c (real, ARM only) and, in tests/mocks/,
            a fake implementation of the exact same function signatures (host only).
tests/      Unity test suite, the mocks, and the vendored Unity framework itself.
app/        The real embedded firmware: drivers/ linked against the real hal/*_kl26z.c files.
Makefile    Host-native test build: `make test` builds and runs the whole suite, no board needed.
.github/workflows/ci.yml   The GitHub Actions workflow that runs `make test` on every push.
README.md   CI badge, build instructions, driver API docs (Section 20's exit criteria).
```

**Why this exact split, and not something simpler:** `drivers/` and `hal/` being separate directories, not one folder, is what makes the boundary in Section 6 enforceable rather than just a convention someone might forget. If `accel_fxos8700.c` lived in the same folder as `hal_i2c_kl26z.c`, it would be easy to accidentally `#include "fsl_i2c.h"` directly from the driver one day and quietly break testability without anyone noticing until the test build failed to compile on a machine without the SDK installed.

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

## 15. Code Structure Explanation

| File | What It Does |
|---|---|
| `hal/hal_i2c.h`, `hal/hal_tsi.h` | The port interfaces (Section 6): function signatures only, no chip-specific types. |
| `hal/hal_i2c_kl26z.c`, `hal/hal_tsi_kl26z.c` | Real implementations, wrapping the SDK's I2C0 and TSI0 drivers exactly as P1 verified. ARM-only, linked only into `app/`. |
| `drivers/accel_fxos8700.c/.h`, `drivers/touch_tsi.c/.h` | Portable sensor logic (Section 7), calling only the HAL functions. Compiled unmodified into both `tests/` and `app/`. |
| `tests/mocks/mock_hal_i2c.c/.h`, `tests/mocks/mock_hal_tsi.c/.h` | Fake implementations of the same HAL interfaces, backed by plain arrays (Section 8). Host-only. |
| `tests/test_accel_fxos8700.c`, `tests/test_touch_tsi.c` | The Unity test suites (Section 9), each with its own `main()`. |
| `tests/unity/` | The vendored, unmodified Unity framework, MIT licensed. |
| `Makefile` | Host-native test build (Section 10). |
| `.github/workflows/ci.yml` | The GitHub Actions workflow (Section 12). |
| `app/main.c` | The real firmware (Section 14), linking the portable drivers against the real HAL. |

## 16. Sample Output

Host test run (`make test`):

```
6 Tests 0 Failures 0 Ignored
OK
5 Tests 0 Failures 0 Ignored
OK
```

Real board (`app/`, over UART):

```
=== P4-C driver library reference (app/) ===
Accelerometer found at I2C address 0x1D
Touch channel 9 calibrated, baseline = 1487

Accel X=  -12 Y=   34 Z=  987 mg | touch=no
Accel X=  -10 Y=   36 Z=  991 mg | touch=yes
```

GitHub, after pushing: the repository's Actions tab shows a green checkmark next to the latest commit, and the README's CI badge (once `OWNER/REPO` is filled in with the real repository path) renders as a green "passing" pill instead of a broken image link.

## 17. Session Plan (maps to Guideline Section 4.6)

| Meeting | Phase | What Members Do | Deliverable | Slide Focus |
|---|---|---|---|---|
| 1 of 2 | Repo structure, HAL design, first test | Set up the repo with `drivers/`, `hal/`, `tests/`, `app/` (Section 5). Define the FXOS8700CQ HAL and driver API headers (Sections 6 to 7). Write the first Unity test with a mocked I2C read (Sections 8 to 9). Get it compiling and passing on the host via the Makefile. | Repo on GitHub. HAL header defined. One passing Unity test running on host PC via Makefile. | Screenshot of the passing test's terminal output; the repo's folder structure; what the group's HAL interface looks like and why. |
| Independent (before Nov 19) | Full driver coverage and CI pipeline | Write the TSI driver with its HAL wrapper and tests. Write I2C driver tests covering error cases (Section 9). Add the GitHub Actions YAML (Section 12). Confirm all tests pass on every push. Write the README with test instructions (Section 20's exit criteria). | All tests green in GitHub Actions on every push. CI badge in README. Two driver modules with full test coverage. | Live demo is the whole slide: push a small change and show the CI check turning green in real time; be ready to explain the mock, not just that it exists. |

## 18. Milestones and Success Criteria

| Milestone | Success Criteria | Evidence |
|---|---|---|
| Repo structure and HAL header | `drivers/`, `hal/`, `tests/`, `app/` exist; `hal_i2c.h` defines a chip-agnostic interface | Code review by leader |
| First passing test (Session 1 exit criteria) | `make test` builds and passes at least one accelerometer test | Terminal screenshot |
| Full accelerometer coverage | Success path, wrong-`WHO_AM_I` path, no-response path, and I2C-failure path all tested | Code review |
| Touch driver and tests | `Touch_Init`/`Touch_IsPressed` implemented and tested, including the below-baseline edge case | Code review plus test output |
| CI pipeline live | A real push to GitHub triggers Actions, and the check passes | Screenshot of the Actions tab |
| Full pipeline (final exit criteria) | Repo has the full `drivers/`/`hal/`/`tests/`/`app/` structure; Unity suite covers both drivers; GitHub Actions runs on every push with all tests passing; README has a CI badge, build instructions, and driver API docs | Live demo: push a change, show the check go green |

## 19. Project-Specific Debugging Reference

| # | Common Problem | Suggested Debugging Steps | Difficulty |
|---|---|---|---|
| 1 | `make test` fails with "undeclared identifier 'NULL'" or similar | A driver or mock file is missing a standard header (`<stddef.h>` for `NULL`, `<stdbool.h>` for `bool`); unlike embedded builds, a bare `-std=c99` host build doesn't pull these in transitively the way SDK headers sometimes do. Add the missing `#include`. | Easy |
| 2 | Tests compile but a driver test always fails, even for what should be the success case | Check the mock was actually programmed with `MockHalI2c_SetReadResponse()` for the *exact* address and register the driver will ask for; the mock returns NACK for anything it wasn't told about, which looks identical to "no sensor present" if a test forgets to program a response. | Medium |
| 3 | A test passes locally but the group is unsure if CI will actually catch a regression | Deliberately break something (change an expected value, like Section 10 did) and run `make test` locally first; if it fails as expected, the same command will fail the same way in Actions. Don't trust a CI pipeline that's never been seen to fail. | Easy |
| 4 | GitHub Actions shows a red X, but `make test` passes locally | Almost always an environment difference: confirm `ubuntu-latest`'s `gcc` isn't warning about something your local compiler doesn't (check the Actions log directly, it shows the exact same compiler output you'd see locally), or that a file wasn't accidentally left out of the commit (`git status` before pushing). | Medium |
| 5 | `app/` fails to build with "accel_fxos8700.h: No such file or directory" | The build's include paths need `hal/` and `drivers/` added explicitly; unlike the host Makefile (which has `-I` flags for every folder), a CMake-based ARM build needs matching `include_directories()` entries for any non-standard folder layout. | Medium |
| 6 | CI badge in the README shows "no status" or a broken image | The badge URL needs the real `OWNER/REPO` path substituted in; it can't resolve until the repository has actually been pushed to GitHub with at least one Actions run completed. | Easy |
| 7 | A group member's PR merges despite a failing CI check | This is a process gap, not a code bug: GitHub can be configured to require passing checks before allowing a merge (branch protection rules), which is worth setting up once a repo's CI is confirmed working, so "merge with a red check" becomes impossible rather than just discouraged. | Medium |

## 20. Glossary

- **Hardware Abstraction Layer (HAL):** a thin layer hiding chip-specific register access behind a stable function API, so code above it doesn't need to change when the hardware underneath does.
- **Mock:** a fake, fully test-controlled stand-in for a real dependency (here, hardware), used so tests can run without that dependency actually being present.
- **Unit test:** an automated check of one specific piece of code's behavior, independent of the rest of the system.
- **Unity:** the small, dependency-free C testing framework this project vendors and uses.
- **Continuous Integration (CI):** automatically running a test suite on a server every time code changes, rather than relying on developers to remember to run it themselves.
- **CI badge:** a small, auto-updating image in a README showing the current pass/fail status of a repository's CI pipeline.
- **Vendoring:** copying a dependency's source directly into your own repository, rather than fetching it at build time or requiring a separate install.
- **Pull request (PR):** a proposed change, pushed to a branch, submitted for review and (ideally) an automated CI check before being merged into the main branch.

## 21. References

- ThrowTheSwitch/Unity (`github.com/ThrowTheSwitch/Unity`): the real, upstream source this project's `tests/unity/` vendors, MIT licensed.
- GitHub Actions documentation (`docs.github.com/actions`): the authoritative reference for workflow YAML syntax beyond this project's minimal example.
- `P1_Project_Manual.md` (this repository): the accelerometer register sequence and touch threshold logic this project's drivers reuse without re-deriving.

## 22. Developer Notes

- **This project's test suite was actually run, not just written.** `make test` was executed in the environment this manual was authored in: all 11 tests passed. A test was then deliberately broken and confirmed to fail correctly, with a non-zero exit code, before being reverted, the strongest confidence available short of a real GitHub Actions run against a pushed repository. `app/` was compiled successfully against the real SDK (18 KB of 128 KB flash), linking the identical driver source files the tests exercise. Neither `app/` has been flashed to physical hardware, nor has this repository been pushed to GitHub to confirm Actions actually goes green there; a project leader should do both before WS8.
- The `README.md`'s CI badge references a placeholder `OWNER/REPO`; each group needs to replace this with their own repository's real path once it exists, or the badge will show as broken indefinitely.
- If a future project wants to extend this pattern (a third driver, a different sensor), the extension point is exactly Sections 6 through 8: define the HAL interface first, write the real and mock implementations, then the portable driver on top. The `tests/`/`app/` split and the CI workflow need no changes at all to accommodate a new driver.

---
*IEEE Texas State University Student Branch. Connect. Build. Inspire.*
