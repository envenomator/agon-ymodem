#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

int serial_open(const char *path, int baud) {
    int fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
        return -1;

    struct termios tio;
    if (tcgetattr(fd, &tio) != 0)
        goto error;

    cfmakeraw(&tio);

    tio.c_cflag |= CLOCAL | CREAD;
    tio.c_cflag &= ~CRTSCTS;   // no HW flow control
    tio.c_cflag &= ~CSTOPB;    // 1 stop bit
    tio.c_cflag &= ~PARENB;    // no parity

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

    tio.c_cc[VMIN]  = 0;
    tio.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tio) != 0)
        goto error;

    // macOS needs this to avoid blocking forever
    ioctl(fd, TIOCEXCL);

    // Switch back to blocking if desired
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);

    return fd;

error:
    close(fd);
    return -1;
}

void serial_close(int fd) {
    close(fd);
}

ssize_t serial_write(int fd, const void *buf, size_t len) {
    return write(fd, buf, len);
}

ssize_t serial_read(int fd, void *buf, size_t len) {
    return read(fd, buf, len);
}
