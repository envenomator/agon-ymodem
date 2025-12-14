#ifdef __linux__

#include <libudev.h>
#include <string.h>
#include "serial_enum.h"

int serial_enumerate_linux(
    serial_device_t *out,
    int max_devices
) {
    struct udev *udev = udev_new();
    if (!udev) return -1;

    struct udev_enumerate *e = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(e, "tty");
    udev_enumerate_scan_devices(e);

    int count = 0;
    struct udev_list_entry *entry;

    udev_list_entry_foreach(
        entry, udev_enumerate_get_list_entry(e)) {

        if (count >= max_devices)
            break;

        struct udev_device *d =
            udev_device_new_from_syspath(
                udev,
                udev_list_entry_get_name(entry));

        const char *node = udev_device_get_devnode(d);
        if (!node) goto next;

        serial_device_t *dev = &out[count];
        memset(dev, 0, sizeof(*dev));

        strncpy(dev->devnode, node, sizeof(dev->devnode) - 1);

        const char *vid =
            udev_device_get_property_value(d, "ID_VENDOR_ID");
        const char *pid =
            udev_device_get_property_value(d, "ID_MODEL_ID");
        const char *ser =
            udev_device_get_property_value(d, "ID_SERIAL_SHORT");

        if (vid) strncpy(dev->vendor_id, vid, sizeof(dev->vendor_id) - 1);
        if (pid) strncpy(dev->product_id, pid, sizeof(dev->product_id) - 1);
        if (ser) strncpy(dev->serial, ser, sizeof(dev->serial) - 1);

        count++;

    next:
        udev_device_unref(d);
    }

    udev_enumerate_unref(e);
    udev_unref(udev);
    return count;
}

#endif

