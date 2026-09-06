# P4-C: Reference

*Part of the P4-C manual split. See `P4C_1_Start_Here.md` for the full file list and how to use this manual.*

---

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
- P1's manual (`P1_Sensor_Dashboard/`, this repository): the accelerometer register sequence and touch threshold logic this project's drivers reuse without re-deriving.

## 22. Developer Notes

- **This project's test suite was actually run, not just written.** `make test` was executed in the environment this manual was authored in: all 11 tests passed. A test was then deliberately broken and confirmed to fail correctly, with a non-zero exit code, before being reverted, the strongest confidence available short of a real GitHub Actions run against a pushed repository. `app/` was compiled successfully against the real SDK (18 KB of 128 KB flash), linking the identical driver source files the tests exercise. Neither `app/` has been flashed to physical hardware, nor has this repository been pushed to GitHub to confirm Actions actually goes green there; a project leader should do both before WS8.
- The `README.md`'s CI badge references a placeholder `OWNER/REPO`; each group needs to replace this with their own repository's real path once it exists, or the badge will show as broken indefinitely.
- If a future project wants to extend this pattern (a third driver, a different sensor), the extension point is exactly Sections 6 through 8: define the HAL interface first, write the real and mock implementations, then the portable driver on top. The `tests/`/`app/` split and the CI workflow need no changes at all to accommodate a new driver.

---
*IEEE Texas State University Student Branch. Connect. Build. Inspire.*
