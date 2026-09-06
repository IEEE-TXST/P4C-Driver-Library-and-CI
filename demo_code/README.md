# P4-C Demo Code

Reference material only. Design your own `drivers/`/`hal/`/`tests/`/`app/` split and get one
test passing yourself first; open this only if you get stuck.

`01_driver_library_ci_repo/` is a complete, working repository, structured exactly the way a
group would push it to GitHub: `drivers/`, `hal/`, `tests/`, `app/`, a root `Makefile`, a
`.github/workflows/ci.yml`, and a `README.md` with build instructions and driver API docs (see
that `README.md` for the full explanation of why the repo is laid out this way). It isn't a
sketch, it's the actual reference this manual's Sections 6 through 11 walk through line by
line.

**This was genuinely tested, both ways, not just written and assumed correct.**

- The host-native test suite (`make test`) was actually run, repeatedly, in the environment
  this was authored in: all 11 Unity tests pass. A test was also deliberately broken (one
  expected value changed) to confirm the suite actually catches a wrong answer and `make`
  exits non-zero, exactly what would fail a GitHub Actions run, then reverted. This is the
  strongest verification available without literally pushing to GitHub and watching Actions
  run, since `make test` is the exact command the CI workflow calls.
- The real embedded firmware (`app/`) was compiled successfully against
  `SDK_2_2_0_FRDM-KL26Z` (18 KB of the 128 KB flash budget). It links the *exact same*
  `drivers/accel_fxos8700.c` and `drivers/touch_tsi.c` source files the host tests exercise,
  now compiled against the real `hal/hal_i2c_kl26z.c` and `hal/hal_tsi_kl26z.c` instead of the
  mocks, which is the entire point of the HAL boundary: one driver, two completely different
  hardware realities, zero changes to the driver code itself.

The Unity test framework (`tests/unity/`) is the real, unmodified upstream source (MIT
licensed, `tests/unity/LICENSE.txt` included), vendored directly into the repo rather than
fetched at build time, so `make test` works offline and CI needs no extra setup step to obtain
it.

`app/` has not been flashed to physical hardware, and `.github/workflows/ci.yml` has not
actually run on GitHub's servers (this repo hasn't been pushed anywhere yet). A project leader
should do both before WS8: flash `app/` to a real board and confirm the printed sensor readings
look sane, and push this repo to a real GitHub repository and confirm the Actions tab shows a
green check, not just a plausible-looking YAML file.
