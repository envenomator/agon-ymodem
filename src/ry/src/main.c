#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <agon/mos.h>
#include <string.h>
#include <agon/timer.h>
#include <ez80f92.h>
#include "serial.h"
#include "crc32.h"
#include "filesize.h"

#define MAXDIRLENGTH                   256
#define MAXNAMELENGTH                  100
#define YMODEM_RECEIVE                 1
#define YMODEM_SEND                    2
#define YMODEM_PACKET_1K_SIZE          1024
#define YMODEM_PACKET_HEADER           3
#define YMODEM_PACKET_TRAILER          2

#define MAXDEBUGLIST                  15

uint32_t readint(void) {
  uint32_t result;

  result = getbyte();
  result = result | ((uint32_t)getbyte() << 8);
  result = result | ((uint32_t)getbyte() << 16);
  result = result | ((uint32_t)getbyte() << 24);

  return result;
}

void writeint(uint32_t value) {
    putch((uint8_t)(value & 0xFF));
    putch((uint8_t)((value >> 8) & 0xFF));
    putch((uint8_t)((value >> 16) & 0xFF));
    putch((uint8_t)((value >> 24) & 0xFF));
}

unsigned int getCWD(char *dir, int maxlength) {
  unsigned int pathlength;

  ffs_getcwd(dir, maxlength);
  pathlength = strlen(dir);
  if(dir[pathlength-1] != '/') {
    strcat(dir, "/");
    pathlength++;
  }

  return pathlength;
}

bool set_VDP_ymodem(uint8_t direction) {
  putch(23);
  putch(28);
  putch(direction);
  if(getbyte() != 'C') return false;

  return true;
}

char stringlist[MAXDEBUGLIST][256];
int namelengthlist[MAXDEBUGLIST];

int get_files(const char *path) {
  unsigned int filename_length;
  unsigned int file_length;
  unsigned int packet_length;
  unsigned filenumber;
  uint8_t state;
  char filename[MAXNAMELENGTH];
  char mosfilename[MAXDIRLENGTH+MAXNAMELENGTH+1];
  char *ptr;
  uint8_t mosfh;
  uint8_t buffer[YMODEM_PACKET_1K_SIZE];
  uint32_t crc32_target, crc32_result;

  // DEBUG
  for(int i = 0; i < 3; i++ ){
    strcpy(stringlist[i], "");
    namelengthlist[i] = 0;
  }
  // DEBUG END
  if(!set_VDP_ymodem(YMODEM_RECEIVE)) return 0;

  filenumber = 0;
  mosfh = 0;

  while(1) {
    state = getbyte();
    switch(state) {
      case 0:
        return filenumber;
        break;
      case 1: // Header
        // Get filename
        ptr = (char*)buffer;
        filenumber++;
        filename_length = readint();
        getblock(filename, filename_length);
        filename[filename_length] = 0;
        file_length = readint();
        // DEBUG
        if(filenumber < MAXDEBUGLIST) {
          strcpy(stringlist[filenumber-1], filename);
          namelengthlist[filenumber-1] = file_length;
        }
        // DEBUG END
        strcpy(mosfilename, path);
        strcat(mosfilename, filename);
        mosfh = mos_fopen(mosfilename, FA_WRITE | FA_CREATE_ALWAYS);
        crc32_initialize();
        putch('S'); // sync
        putch('1');
        break;
      case 2: // Data packet
        packet_length = readint();
        getblock(ptr, packet_length);
        crc32(ptr, packet_length);
        mos_fwrite(mosfh, ptr, packet_length);
        putch('S'); // sync
        putch('2');
        break;
      case 3: // Check CRC
        crc32_target = readint();      
        crc32_result = crc32_finalize(); 
        putch('S'); // sync
        if(crc32_target != crc32_result) {
          putch('X'); // Abort
          mos_fclose(mosfh);
          mos_del(filename);
          filenumber--;
          return filenumber;
        }
        putch('V'); // Verified
        break;
      case 4: // End-of-transmission (file)
        mos_fclose(mosfh);
        putch('S'); // sync
        putch('4');
        break;
      case 0xff:
      default:
        if(filenumber) {
          mos_fclose(mosfh);
          mos_del(filename);
          filenumber--;
        }
        return filenumber;
    }
  }
}

char *get_base_dir(char *path) {
    int len = strlen(path);
    for (int i = len - 1; i >= 0; i--) {
        if (path[i] == '/' || path[i] == '\\') {
            path[i] = '\0';   // terminate before the slash
            return path;
        }
    }
    return path; // no slash found
}

void print_usage(void) {
  printf("Usage: ry [directory]\r\n\r\n");
}

int main(int argc, char **argv) {
  char dir[MAXDIRLENGTH+1];
  int filenumber;

  sysvar_init();

  if(argc > 2) {
    print_usage();
    return 0;
  }

  if(argc == 2) {
    if(mos_isdirectory(argv[1]) != 0) {
      printf("Invalid path\r\n");
      return 0;
    }
    strcpy(dir, argv[1]);
    if(dir[strlen(dir)-1] != '/') strcat(dir, "/");
  }
  else strcpy(dir, "./");

  filenumber = get_files(dir);

  return 0;
}
