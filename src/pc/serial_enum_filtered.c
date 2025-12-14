#include "serial_enum.h"
#include <string.h>
#include <stdio.h>

int serial_enumerate_filtered( serial_device_t *out, int max_devices) {
    if (!out || max_devices <= 0)
        return -1;

    serial_device_t tmp[64];   // internal scratch buffer
    int total = serial_enumerate(tmp, 64);

    if (total < 0)
        return -1;

    int count = 0;
    for (int i = 0; i < total && count < max_devices; i++) {
        const char *name = tmp[i].devnode;

        if (!name)
            continue;

        if (strstr(name, "usbserial") ||
            strstr(name, "ttyUSB") ||
            strstr(name, "ttyACM")) {
            out[count++] = tmp[i];  // struct copy
        }
    }

    return count;
}

int serial_autodetect( char *devicename ) {
  serial_device_t filtered[16];
  int n = serial_enumerate_filtered(filtered, 16);

  devicename[0] = 0;

  switch(n) {
    case 0:
      printf("No USB-Serial devices found\n"); break;
    case 1:
      strcpy(devicename, filtered[0].devnode);
      printf("Autodetected: %s\n", devicename);
      break;
    default:
      printf("Multiple devices found:\n"); 
      for(int i = 0; i < n; i++) {
        printf("%d - %s\n", i, filtered[i].devnode);
      }
      printf("\nSelect a device with the -d option\n");
  }
  return n;
}
