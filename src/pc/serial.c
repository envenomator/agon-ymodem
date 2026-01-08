/*
 * serial.c
 *
 * Cross-platform serial port wrapper
 *
 * Supports:
 *   - Linux
 *   - macOS
 *   - Windows
 *
 * Copyright (c) 2026 Jeroen Venema
 */

#include "serial.h"

#include <string.h>
#include <errno.h>

#ifdef _WIN32

/* ============================== WINDOWS ============================== */

#include <windows.h>
#include <stdio.h>

serial_handle_t serial_open(const char *path, int baud)
{
    char device[32];

    /* Windows requires \\.\COMx for COM10+ */
    if (strncmp(path, "\\\\.\\", 4) == 0)
        strncpy(device, path, sizeof(device));
    else
        snprintf(device, sizeof(device), "\\\\.\\%s", path);

    HANDLE h = CreateFileA(
        device,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (h == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;

    DCB dcb;
    SecureZeroMemory(&dcb, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(h, &dcb))
        goto error;

    dcb.BaudRate = baud;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    dcb.fBinary = TRUE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl  = DTR_CONTROL_ENABLE;
    dcb.fRtsControl  = RTS_CONTROL_ENABLE;

    if (!SetCommState(h, &dcb))
        goto error;

    COMMTIMEOUTS tmo;
    memset(&tmo, 0, sizeof(tmo));
    tmo.ReadIntervalTimeout = MAXDWORD;
    SetCommTimeouts(h, &tmo);

    return h;

error:
    CloseHandle(h);
    return INVALID_HANDLE_VALUE;
}

void serial_close(serial_handle_t h)
{
    CloseHandle(h);
}

size_t serial_read(serial_handle_t h, void *buf, size_t len)
{
    DWORD r = 0;
    if (!ReadFile(h, buf, (DWORD)len, &r, NULL))
        return -1;
    return (size_t)r;
}

size_t serial_write(serial_handle_t h, const void *buf, size_t len)
{
    DWORD w = 0;
    if (!WriteFile(h, buf, (DWORD)len, &w, NULL))
        return -1;
    return (size_t)w;
}

#else

/* ========================== LINUX / macOS ========================== */

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

serial_handle_t serial_open(const char *path, int baud)
{
    int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
        return -1;

    struct termios tio;
    if (tcgetattr(fd, &tio) != 0)
        goto error;

#ifdef __linux__
    cfmakeraw(&tio);
#else
    /* macOS compatibility */
    tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP |
                     INLCR | IGNCR | ICRNL | IXON);
    tio.c_oflag &= ~OPOST;
    tio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tio.c_cflag &= ~(CSIZE | PARENB);
    tio.c_cflag |= CS8;
#endif

    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~PARENB;

#ifdef CRTSCTS
    tio.c_cflag &= ~CRTSCTS;
#endif

    speed_t speed;
    switch (baud) {
        case 9600:   speed = B9600; break;
        case 19200:  speed = B19200; break;
        case 38400:  speed = B38400; break;
        case 57600:  speed = B57600; break;
        case 115200: speed = B115200; break;
        default:
            errno = EINVAL;
            goto error;
    }

    cfsetispeed(&tio, speed);
    cfsetospeed(&tio, speed);

    /* Blocking reads */
    tio.c_cc[VMIN]  = 1;
    tio.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tio) != 0)
        goto error;

#ifdef __APPLE__
    ioctl(fd, TIOCEXCL);
#endif

    /* Switch back to blocking */
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);

    return fd;

error:
    close(fd);
    return -1;
}

void serial_close(serial_handle_t h)
{
    close(h);
}

size_t serial_read(serial_handle_t h, void *buf, size_t len)
{
    return read(h, buf, len);
}

size_t serial_write(serial_handle_t h, const void *buf, size_t len)
{
    return write(h, buf, len);
}

#endif

