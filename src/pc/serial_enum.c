#include "serial_enum.h"

#ifdef __linux__
int serial_enumerate_linux(serial_device_t *, int);
#define serial_enumerate_platform serial_enumerate_linux
#elif defined(__APPLE__)
int serial_enumerate_macos(serial_device_t *, int);
#define serial_enumerate_platform serial_enumerate_macos
#else
#error Unsupported platform
#endif

int serial_enumerate(
    serial_device_t *out,
    int max_devices
) {
    if (!out || max_devices <= 0)
        return -1;

    return serial_enumerate_platform(out, max_devices);
}

