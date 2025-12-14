#pragma once
#include <unistd.h>

int serial_open(const char *path, int baud);
void serial_close(int fd);
ssize_t serial_write(int fd, const void *buf, size_t len);
ssize_t serial_read(int fd, void *buf, size_t len);
