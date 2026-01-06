#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h> 
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>

#include "ymodem.h"
#include "serial.h"
#include "serial_enum_filtered.h"

#define DEFAULT_BAUDRATE        115200

void usage(const char *progname) {
  printf("Usage:\n");
  printf("  %s [-b baudrate] [-d device] -r [directory]       Receive mode, optional target directory\n", progname);
  printf("  %s [-b baudrate] [-d device] -s file1 [file2 ...] Send mode, at least one file required\n", progname);
}

int is_directory(const char *path) {
#ifdef _WIN32
    struct _stat st;
    if (_stat(path, &st) != 0) return 0;
    return (st.st_mode & _S_IFDIR) != 0;
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
#endif
}

int main(int argc, char** argv) {
  char devicename[NAME_MAX + 1];
  char *dir;
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
      if(receive) { usage(basename(argv[0])); return -1;}
      send = true;
      break;
    case 'r':
      if(send) { usage(basename(argv[0])); return -1;}
      receive = true;
      break;
    case 'h':
    default:
      usage(basename(argv[0]));
      return 0;
    }
  }

  if(!send && !receive) { usage(basename(argv[0])); return 0; }
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
      usage(basename(argv[0]));
      return -1;
    }
    ymodem_send(serial_port, filecount, filenames);
  }

  if(receive) {
    if(filecount > 1) {
      usage(basename(argv[0]));
      return -1;
    }
    if(filecount == 1) {
      if(is_directory(filenames[0]) == 0) {
        printf("Invalid path \'%s\'\n", filenames[0]);
        return 0;
      }
      dir = malloc(strlen(filenames[0]) + 2); // allowing possible extra '/'
      if(!dir) {
        printf("Memory allocation error\n");
        return 0;
      }
      strcpy(dir, filenames[0]);
      if(dir[strlen(dir)-1] != '/') strcat(dir, "/");

      printf("Receive path %s\n", dir);
    }
    else {
      dir = malloc(3);
      strcpy(dir, "./");
    }
    ymodem_receive(serial_port, dir);
    free(dir);
  }

  // Clean-up
  close(serial_port);
  return 0;
}

