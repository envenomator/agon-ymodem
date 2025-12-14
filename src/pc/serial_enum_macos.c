#ifdef __APPLE__

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <string.h>
#include "serial_enum.h"

int serial_enumerate_macos(
    serial_device_t *out,
    int max_devices
) {
    CFMutableDictionaryRef match =
        IOServiceMatching(kIOSerialBSDServiceValue);

    CFDictionarySetValue(
        match,
        CFSTR(kIOSerialBSDTypeKey),
        CFSTR(kIOSerialBSDAllTypes)
    );

    io_iterator_t iter;
    if (IOServiceGetMatchingServices(
            kIOMainPortDefault,
            match,
            &iter) != KERN_SUCCESS)
        return -1;

    int count = 0;
    io_object_t obj;

    while ((obj = IOIteratorNext(iter))) {
        if (count >= max_devices)
            break;

        serial_device_t *dev = &out[count];
        memset(dev, 0, sizeof(*dev));

        CFTypeRef path =
            IORegistryEntryCreateCFProperty(
                obj,
                CFSTR(kIOCalloutDeviceKey),
                kCFAllocatorDefault,
                0);

        if (path) {
            CFStringGetCString(
                path,
                dev->devnode,
                sizeof(dev->devnode),
                kCFStringEncodingUTF8);
            CFRelease(path);
            count++;
        }

        IOObjectRelease(obj);
    }

    IOObjectRelease(iter);
    return count;
}

#endif

