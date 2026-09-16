# P4-C: Embedded Driver Library and CI Pipeline

*Hardware abstraction, portable driver API, unit testing, and automated CI.*

IEEE TXST Student Branch, Fall 2026 FRDM-KL26Z project series. Capstone option, 2 independent groups.

**Platform:** NXP FRDM-KL26Z (ARM Cortex-M0+, 48 MHz, 128 KB Flash). Prerequisite: P1 (all peripherals), P2, P3 complete. GitHub account required.

## What this project is

Members design and implement a hardware abstraction layer (HAL) for the FRDM-KL26Z, wrapping the FXOS8700CQ I2C driver and the TSI capacitive touch driver behind clean portable APIs. The drivers are tested using the Unity C testing framework with mocked hardware registers, tests run on a host PC, not the board. A GitHub Actions workflow runs the full test suite on every push. This project is about software engineering discipline: separating platform code from portable code, writing testable firmware, and building a CI pipeline that catches regressions automatically.

## Exit criteria

GitHub repo with `drivers/`, `hal/`, `tests/`, `app/` structure. Unity test suite covering the FXOS8700CQ and TSI drivers. GitHub Actions CI running on every push with all tests passing. README with CI badge, build instructions, and driver API documentation.

## Repo layout

- `P4C_1_Start_Here.md` through `P4C_4_Reference.md`: the Project Manual, split into four files.
- `demo_code/01_driver_library_ci_repo/`: the actual driver library repo structure this project's exit criteria describes. **Has its own README** with the CI badge, build instructions, and driver API documentation, since that's what a member would see if this folder were pushed as its own standalone repository. See [`demo_code/01_driver_library_ci_repo/README.md`](demo_code/01_driver_library_ci_repo/README.md).
  - `drivers/`: portable sensor drivers (`accel_fxos8700.c`, `touch_tsi.c`), never touch a register directly.
  - `hal/`: the hardware boundary, real KL26Z implementations.
  - `tests/`: Unity test suite plus `tests/mocks/`, host-native, no board required.
  - `app/`: the real embedded firmware, links `drivers/` against the real `hal/` implementations.

## Getting started

Read `P4C_1_Start_Here.md` first, then `P4C_3_Setup_and_Walkthrough.md`. Run `make test` inside `demo_code/01_driver_library_ci_repo/` to see the host-native test suite pass without any board attached.

## Resume line

> Developed portable embedded driver library for ARM Cortex-M0+ with hardware-mocked unit testing using Unity framework and GitHub Actions CI; test suite runs on host PC without hardware.
