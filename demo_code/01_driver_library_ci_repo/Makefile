# Host-native test build. No arm-none-eabi-gcc, no SDK, no board: this is
# the exact command GitHub Actions runs on every push (see
# .github/workflows/ci.yml). The real embedded firmware (app/) has its own
# separate build under app/armgcc/, using the ARM cross-compiler; see the
# manual, Section 12.

CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -Werror \
          -Itests/unity -Ihal -Idrivers -Itests/mocks
BUILD_DIR := build

UNITY_SRC := tests/unity/unity.c

ACCEL_TEST_SRCS := tests/test_accel_fxos8700.c drivers/accel_fxos8700.c \
                    tests/mocks/mock_hal_i2c.c $(UNITY_SRC)
TOUCH_TEST_SRCS := tests/test_touch_tsi.c drivers/touch_tsi.c \
                    tests/mocks/mock_hal_tsi.c $(UNITY_SRC)

.PHONY: test clean

test: $(BUILD_DIR)/test_accel_fxos8700 $(BUILD_DIR)/test_touch_tsi
	$(BUILD_DIR)/test_accel_fxos8700
	$(BUILD_DIR)/test_touch_tsi

$(BUILD_DIR)/test_accel_fxos8700: $(ACCEL_TEST_SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(ACCEL_TEST_SRCS)

$(BUILD_DIR)/test_touch_tsi: $(TOUCH_TEST_SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(TOUCH_TEST_SRCS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
