#include "serial_enum_filtered.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h> 
#include <errno.h>
#include <string.h>
#include "ymodem.h"
#include "serial.h"

int main(int argc, char** argv) {
  char devicename[256];
  int serial_port, opt;
  const char *device = devicename;
  int baud = 115200;
  bool auto_device = true;
  bool send = true;

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
    case 's': break;
    case 'r':
      send = false;
      break;
    case 'h':
    default:
      printf("Usage: %s [-s | -r] <-d device> <-b baud> [file(s)]\n", argv[0]);
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

  // Diagnostics
  //printf("device=%s baud=%d send=%d\n", device, baud, send);  
  //printf("Files:\n");
  //for (int i = optind; i < argc; i++) {
  //    printf("File %d: %s\n", i - optind + 1, argv[i]);
  //}

  int filecount = argc - optind;
  char **filenames = &argv[optind];

  if(send) {
    if(filecount <= 0) {
      printf("No input files\n");
      return -1;
    }
    ymodem_send(serial_port, filecount, filenames);
  }
  else {
    ymodem_receive(serial_port);
  }
  // Clean-up
  close(serial_port);
  return 0;
}

