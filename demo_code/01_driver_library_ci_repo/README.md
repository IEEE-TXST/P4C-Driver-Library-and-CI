# FRDM-KL26Z Driver Library

<!-- Once this repo is pushed to GitHub, replace OWNER/REPO below with the
     real path so this badge shows live status instead of a broken image. -->
![CI](https://github.com/OWNER/REPO/actions/workflows/ci.yml/badge.svg)

A portable driver library for the on-board FXOS8700CQ accelerometer and TSI capacitive touch
sensor on the FRDM-KL26Z, with a host-native Unity test suite that runs on every push via
GitHub Actions, no board required.

## Why this repo is laid out this way

```
drivers/    Portable sensor drivers. Talk only to hal/*.h, never to a register directly.
hal/        The hardware boundary. hal_*_kl26z.c (real, ARM only) and, in tests/mocks/,
            a fake implementation of the exact same functions (host only).
tests/      Unity test suite plus the mocks. Runs natively on your laptop; no board involved.
app/        The real embedded firmware. Links drivers/ against the real hal/*_kl26z.c files
            and runs on actual hardware.
```

The same `drivers/accel_fxos8700.c` and `drivers/touch_tsi.c` files are compiled into both
`tests/` (against the mocks, running on your laptop) and `app/` (against real hardware,
running on the board). If the tests pass, you have real confidence the driver logic is
correct, independent of whether you currently have a board plugged in.

## Running the tests

```bash
make test
```

No board, no ARM toolchain, no SDK needed for this command; it only needs a native C compiler
(`gcc` or `clang`) and builds two small executables that run directly on your machine. This is
the exact command `.github/workflows/ci.yml` runs on every push and pull request.

## Building the real firmware

See `app/armgcc/` and the P0 project manual (Section 8) for the general `cmake` / `make` /
`objcopy` flow used throughout this project series.

## Driver API

### `drivers/accel_fxos8700.h`

```c
bool Accel_Init(accel_handle_t *handle);
bool Accel_ReadXYZ_mg(const accel_handle_t *handle, int16_t *xMg, int16_t *yMg, int16_t *zMg);
```

`Accel_Init()` probes the four possible I2C addresses, confirms the sensor via its `WHO_AM_I`
register, and configures it for `+/-4g` active mode. `Accel_ReadXYZ_mg()` returns the current
acceleration on all three axes in milli-g. Both return `false` on any I2C failure.

### `drivers/touch_tsi.h`

```c
void Touch_Init(touch_handle_t *handle, uint32_t channel);
bool Touch_IsPressed(const touch_handle_t *handle);
```

`Touch_Init()` calibrates the given TSI channel and stores its untouched baseline.
`Touch_IsPressed()` returns `true` if the channel's current reading is far enough above that
baseline to count as a touch.

## Contributing

Pull requests are the expected workflow: branch, make a change, push, and open a PR. GitHub
Actions runs `make test` automatically on the PR; a reviewer should not approve anything with a
red CI check. See the project manual, Section 13, for more on why this matters.
