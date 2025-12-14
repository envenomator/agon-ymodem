#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define SERIAL_STR_MAX 128

typedef struct {
    char devnode[SERIAL_STR_MAX];   // /dev/ttyUSB0, /dev/cu.usbserial-XXX
    char vendor_id[8];              // "0403", "" if unknown
    char product_id[8];             // "6001", "" if unknown
    char serial[SERIAL_STR_MAX];     // device serial, "" if unknown
} serial_device_t;

/* Enumerate serial devices.
 * Return number of devices found or negative on error.
 */
int serial_enumerate(
    serial_device_t *out,
    int max_devices
);

#ifdef __cplusplus
}
#endif

