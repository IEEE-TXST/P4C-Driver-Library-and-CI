# P4-C: Concepts and Hardware

*Part of the P4-C manual split. See `P4C_1_Start_Here.md` for the full file list and how to use this manual.*

---

## 1. New Concepts You Need Before Starting

Builds on P1 (the accelerometer and touch sensor this project's drivers wrap, reused without re-deriving) and general comfort with C from P2/P3. Everything below is new to P4-C.

**Hardware Abstraction Layer (HAL).** A thin layer of code whose entire job is hiding "how do I actually talk to this specific chip's registers" behind a small, stable set of function calls. Code above the HAL (a driver, an application) calls those functions and never touches a register directly; code below the HAL is the only place that does. The payoff: swap what's below the HAL (a different chip, or, as this project does, no chip at all) and everything above it keeps working unmodified.

**Mocking.** Replacing a real dependency, here, actual hardware, with a fake stand-in that behaves predictably and is fully controlled by the test writer. A mock doesn't try to simulate real I2C electrical behavior; it just remembers "if asked to read register X, return these bytes," using nothing more exotic than a plain array. This is the literal meaning of "replace volatile register pointers with plain variables in test builds": the mock's internal state is genuinely just ordinary C variables, no `volatile`, no memory-mapped anything.

**Unit test.** A small, automated check that one specific piece of code behaves correctly, given specific inputs, independent of everything else in the system. "The accelerometer driver converts these exact raw bytes to this exact milli-g value" is a unit test; "the whole board works" is not, that's an integration or system test, a different (and, for a microcontroller, much harder to automate) thing.

**Unity.** A small, dependency-free C testing framework, purpose-built for exactly this kind of embedded use case: no dynamic memory allocation required, no C++ needed, compiles and runs anywhere a C compiler exists, including directly on the chip if you wanted to (this project doesn't; the tests run on the host). `TEST_ASSERT_EQUAL_INT16(expected, actual)` and similar macros are Unity's entire vocabulary; Section 9 shows them in real use.

**Continuous Integration (CI).** Automatically running your test suite every time code changes, on a server, not manually on your own machine, so nobody can forget, and so a broken test is visible to the whole team, not just whoever happened to run it locally. GitHub Actions is one specific, free CI service tightly integrated with GitHub itself; Section 12 covers the exact workflow file this project uses.

**Repository structure as documentation.** `drivers/`, `hal/`, `tests/`, `app/` isn't an arbitrary folder scheme; each name is a promise about what kind of code lives there and what it's allowed to depend on. A new contributor (or your future self in six months) can navigate this repository correctly without reading a single line of code, purely from the folder names, which is exactly the point.

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

---

**Next:** `P4C_3_Setup_and_Walkthrough.md` for the hands-on session steps.

---
*IEEE Texas State University Student Branch. Connect. Build. Inspire.*
