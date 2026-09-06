# TXST IEEE Student Branch: FRDM-KL26Z Project Series
## Capstone Manual, P4-C: Embedded Driver Library and CI Pipeline

**Document status:** DRAFT v0.1, for project-leader review and bench testing before member use
**Track:** Embedded Systems and Software | **Difficulty:** Medium | **Sessions:** 1 (WS8, Nov 5, 2026), plus independent build before the Nov 19 demo
**Groups:** 2 groups of 3, working independently
**Companion documents:** P0's manual (`P0_Board_Orientation_and_Toolchain_Setup/`, start at `P0_1_Start_Here.md`) through P3's manual (`P3_DMA_ADC_FIR_Filter/`) (read all four first), *TXST IEEE FRDM-KL26Z Project Specification*
**Hardware:** FRDM-KL26Z (only needed for the final `app/` bench test; most of this project runs on a laptop with no board attached)
**Prerequisite:** a GitHub account, free, for every member
**Demo code:** see `demo_code/` in this project's folder

---

## This Manual Is Split Into 4 Files

Long single files invite procrastination. Read only what you need, when you need it:

1. **`P4C_1_Start_Here.md`** (this file) — how to use this manual, why P4-C exists, purpose, prerequisites.
2. **`P4C_2_Concepts_and_Hardware.md`** — background theory (HAL, mocking, unit tests, Unity, CI) and the repository structure reference. Read once if any term below is new to you.
3. **`P4C_3_Setup_and_Walkthrough.md`** — the actual hands-on steps. **This is the file you follow during the session.**
4. **`P4C_4_Reference.md`** — code structure explanation, sample output, session plan, milestones, debugging table, glossary, references, developer notes. Look things up here when stuck.

Section numbers (0-22) are kept consistent across all 4 files, so "see Section 9" always means the same section no matter which file you're in.

---

## How to Use This Manual

Same rule as every prior manual: a reference, not required reading. Build the repo structure and get one test passing yourself first; open `demo_code/` only when stuck.

This project reads differently from every embedded project before it, on purpose. There is comparatively little hardware debugging here (Section 0 explains why), and comparatively more software architecture: how you draw the line between "code that touches a register" and "code that doesn't," and what that line buys you. Take the software engineering content as seriously as the embedded content; for a lot of hiring managers, this is the project on your resume that reads as most immediately recognizable.

Project leaders: bench-test `demo_code/01_driver_library_ci_repo/` before WS8, in both senses this project actually has: run `make test` on your own laptop and confirm all 11 tests pass, and separately, flash `app/` to a real board and confirm the printed accelerometer and touch readings look sane. Also worth doing once, if you have time: actually push the reference repo to a scratch GitHub repository and watch the Actions tab go green, so you've seen the real CI pipeline work, not just read a YAML file that looks correct.

**A note on accuracy:** this project's test suite was not just written, it was run, repeatedly, and a test was deliberately broken and confirmed to fail correctly before being fixed again, the strongest verification available short of literally watching GitHub Actions execute. The real embedded firmware was compiled and confirmed to link the *exact same* driver source files the test suite exercises. Section 21 has the full trail.

---

## 0. Why This Session Exists

Every project before this one asked "does the firmware work." This one asks a different, equally real question: "how do you know your firmware works, and how do you keep knowing that as more people touch the code?" A chapter with 25 boards and rotating membership is exactly the situation where untested, unreviewed embedded code quietly breaks in ways nobody notices until a demo fails. The pattern this project teaches, separate the hardware-touching code from the logic, mock the hardware for testing, automate the test run on every change, is not a student-project simplification of how real companies work; it is, close to verbatim, how real companies work. A firmware engineer who can say "I built a driver library with a mocked test suite and CI" in an interview is describing a genuinely professional practice, not a toy exercise.

## 2. Purpose

By the end of this project, every group has: a GitHub repository laid out as `drivers/`, `hal/`, `tests/`, `app/`; a hardware abstraction layer with a real and a mocked implementation of the same interface; a Unity test suite covering both the accelerometer and touch drivers, including error cases; a GitHub Actions workflow running that suite on every push; and a README with a CI badge, build instructions, and driver API documentation, demonstrated at the Nov 19 showcase with the badge showing green.

## 3. Prerequisites

P1 (all peripherals), P2, and P3 all complete, plus a free GitHub account for every member of the group. This project reuses P1's exact, already-verified accelerometer register sequence and touch threshold logic; if that feels shaky, revisit P1's manual first.

---

**Next:** `P4C_2_Concepts_and_Hardware.md` for the concepts and repository structure reference, or skip straight to `P4C_3_Setup_and_Walkthrough.md` if you're already comfortable with HALs, mocking, and CI.

---
*IEEE Texas State University Student Branch. Connect. Build. Inspire.*
