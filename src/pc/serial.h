#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
typedef HANDLE serial_handle_t;
#else
typedef int serial_handle_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

serial_handle_t serial_open(const char *path, int baud);
void serial_close(serial_handle_t h);

size_t serial_read(serial_handle_t h, void *buf, size_t len);
size_t serial_write(serial_handle_t h, const void *buf, size_t len);

#ifdef __cplusplus
}
#endif

