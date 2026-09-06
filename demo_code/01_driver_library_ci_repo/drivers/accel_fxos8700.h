/*
 * Portable FXOS8700CQ accelerometer driver. Talks to the sensor only
 * through hal_i2c.h, never touches I2C0 or any register directly, which
 * is what makes this exact same .c file compile and run correctly for
 * both the real board (app/, linked against hal_i2c_kl26z.c) and the host
 * test suite (tests/, linked against tests/mocks/mock_hal_i2c.c). See the
 * manual, Section 6.
 */
#ifndef ACCEL_FXOS8700_H
#define ACCEL_FXOS8700_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t address; /* the I2C address the sensor was found at; 0 if not found */
    bool found;
} accel_handle_t;

/* Probes the four possible I2C addresses (0x1C, 0x1D, 0x1E, 0x1F),
   confirms WHO_AM_I on whichever one answers, and if found, brings the
   sensor up in +/-4g active mode (the same register sequence P1
   verified). Returns true if a sensor was found and configured. */
bool Accel_Init(accel_handle_t *handle);

/* Reads X, Y, Z in milli-g. Returns false (and leaves all three outputs
   at 0) if "handle" was never successfully initialized or the read
   itself fails. */
bool Accel_ReadXYZ_mg(const accel_handle_t *handle, int16_t *xMg, int16_t *yMg, int16_t *zMg);

#endif /* ACCEL_FXOS8700_H */
