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

void writeint(uint32_t value) {
    putch((uint8_t)(value & 0xFF));
    putch((uint8_t)((value >> 8) & 0xFF));
    putch((uint8_t)((value >> 16) & 0xFF));
    putch((uint8_t)((value >> 24) & 0xFF));
}

bool set_VDP_ymodem(uint8_t direction) {
  putch(23);
  putch(28);
  putch(direction);
  if(getbyte() != 'C') return false;

  return true;
}

bool check_files(int filecount, char *filelist[]) {
  bool allfiles_exist = true;
  bool allnames_ok = true;

  for(int i = 0; i < filecount; i++) {
    uint8_t mosfh = mos_fopen(filelist[i], FA_READ);
    if(strlen(filelist[i]) > MAXNAMELENGTH) {
      printf("File \'%s\' - name too large\r\n", filelist[i]);
      allnames_ok = false;
    }
    if(mosfh == 0) {
      printf("File \'%s\' does not exist\r\n", filelist[i]);
      allfiles_exist = false;
    }
    else {
      mos_fclose(mosfh);
    }
  }
  return allfiles_exist && allnames_ok;
}

void send_files(int filecount, char *filelist[]) {
  unsigned int filename_length;
  unsigned int filesize;
  char filename[MAXNAMELENGTH+1];
  uint8_t mosfh;
  uint8_t buffer[YMODEM_PACKET_1K_SIZE];

  if(!set_VDP_ymodem(YMODEM_SEND)) return; 

  for(int filenumber = 0; filenumber < filecount; filenumber++) {
    crc32_initialize();
    mosfh = mos_fopen(filelist[filenumber], FA_READ);
    if(mosfh == 0) {
      writeint(0);
      return;
    }
    else writeint(filecount);

    filename_length = strlen(filelist[filenumber]);
    writeint(filename_length);
    strcpy(filename, filelist[filenumber]);
    putblock(filename, filename_length);  // send filename
    filesize = getfilesize(mosfh);
    
    unsigned int write_len;
    writeint(filesize);

    while(filesize) {
      write_len = mos_fread(mosfh, (char*)buffer, YMODEM_PACKET_1K_SIZE);
      putblock((char*)buffer, write_len);
      crc32((char*)buffer, write_len);
      filesize -= write_len;
    }
    writeint(crc32_finalize());
    mos_fclose(mosfh);

    filecount--;
  }

  writeint(0);
  getbyte();
}

void print_usage(void) {
  printf("Usage: sy <filename> [filename ...]\r\n\r\n");
}

int main(int argc, char **argv) {
  char dir[MAXDIRLENGTH+1];
  int filenumber;
  bool receive = true;

  sysvar_init();

  if(argc == 1) {
    print_usage();
    return 0;
  }

  if(!check_files(argc-1, &argv[1])) {
    printf("Send aborted\r\n");
    return 0;
  }
  send_files(argc-1, &argv[1]);

  return 0;
}
