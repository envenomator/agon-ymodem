#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h> 
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>

#include "ymodem.h"
#include "serial.h"
#include "serial_enum_filtered.h"

#define DEFAULT_BAUDRATE        115200

void usage(const char *progname) {
  printf("Usage: %s <-b baud> <-d device> [ -r | -s [file(s)] ]\n", progname);
}

int main(int argc, char** argv) {
  char devicename[NAME_MAX + 1];
  int serial_port, opt;
  const char *device = devicename;
  int baud = DEFAULT_BAUDRATE;
  bool auto_device = true;
  bool send = false;
  bool receive = false;

  // Process options
  while ((opt = getopt(argc, argv, "srd:b:h")) != -1) {
    switch (opt) {
    case 'd':
      device = optarg;
      auto_device = false;
      break;
    case 'b':
      baud = atoi(optarg);
      break;
    case 's': 
      if(receive) { usage(argv[0]); return -1;}
      send = true;
      break;
    case 'r':
      if(send) { usage(argv[0]); return -1;}
      receive = true;
      break;
    case 'h':
    default:
      usage(argv[0]);
      return 0;
    }
  }
  
  // Autodetect devicename if none given as option
  if(auto_device && serial_autodetect(devicename) != 1) return -1;

  // Open serial port
  serial_port = serial_open(device, baud);
  if (serial_port < 0) {
    printf("Error %i from open: %s\n", errno, strerror(errno));
    return -1;
  }

  int filecount = argc - optind;
  char **filenames = &argv[optind];

  if(send) {
    if(filecount <= 0) {
      usage(argv[0]);
      return -1;
    }
    ymodem_send(serial_port, filecount, filenames);
  }

  if(receive) {
    if(filecount > 0) {
      usage(argv[0]);
      return -1;
    }
    ymodem_receive(serial_port);
  }

  // Clean-up
  close(serial_port);
  return 0;
}

