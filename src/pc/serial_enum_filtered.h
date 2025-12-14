#pragma once
#include "serial_enum.h"

int serial_enumerate_filtered( serial_device_t *out, int max_devices);
int serial_autodetect( char *devicename );
