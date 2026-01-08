// serial_enum_windows.c
#include "serial_enum.h"

#ifdef _WIN32

#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <stdio.h>
#include <string.h>

#pragma comment(lib, "setupapi.lib")

static void extract_vid_pid_serial(
    const char *hwid,
    char *vid,
    char *pid,
    char *serial)
{
    // Example HWID:
    // USB\VID_0403&PID_6001\A50285BI
    vid[0] = pid[0] = serial[0] = '\0';

    const char *v = strstr(hwid, "VID_");
    const char *p = strstr(hwid, "PID_");

    if (v) {
        strncpy(vid, v + 4, 4);
        vid[4] = '\0';
    }

    if (p) {
        strncpy(pid, p + 4, 4);
        pid[4] = '\0';
    }

    const char *slash = strchr(hwid, '\\');
    if (slash && slash[1]) {
        strncpy(serial, slash + 1, SERIAL_STR_MAX - 1);
        serial[SERIAL_STR_MAX - 1] = '\0';
    }
}

int serial_enumerate(serial_device_t *out, int max_devices)
{
    if (!out || max_devices <= 0)
        return -1;

    HDEVINFO hDevInfo = SetupDiGetClassDevs(
        &GUID_DEVCLASS_PORTS,
        NULL,
        NULL,
        DIGCF_PRESENT
    );

    if (hDevInfo == INVALID_HANDLE_VALUE)
        return -1;

    int count = 0;
    SP_DEVINFO_DATA devInfo;
    devInfo.cbSize = sizeof(devInfo);

    for (DWORD i = 0;
         SetupDiEnumDeviceInfo(hDevInfo, i, &devInfo) && count < max_devices;
         i++)
    {
        serial_device_t *dev = &out[count];
        memset(dev, 0, sizeof(*dev));

        char friendly[256];
        if (!SetupDiGetDeviceRegistryPropertyA(
                hDevInfo,
                &devInfo,
                SPDRP_FRIENDLYNAME,
                NULL,
                (PBYTE)friendly,
                sizeof(friendly),
                NULL))
        {
            continue;
        }

        // Friendly name example:
        // "USB-SERIAL CH340 (COM3)"
        char *com = strstr(friendly, "(COM");
        if (!com)
            continue;

        com++; // skip '('
        char *end = strchr(com, ')');
        if (!end)
            continue;

        size_t len = end - com;
        if (len >= SERIAL_STR_MAX)
            len = SERIAL_STR_MAX - 1;

        memcpy(dev->devnode, com, len);
        dev->devnode[len] = '\0';

        // --- Hardware ID for VID/PID/SERIAL ---
        char hwid[256];
        if (SetupDiGetDeviceRegistryPropertyA(
                hDevInfo,
                &devInfo,
                SPDRP_HARDWAREID,
                NULL,
                (PBYTE)hwid,
                sizeof(hwid),
                NULL))
        {
            extract_vid_pid_serial(
                hwid,
                dev->vendor_id,
                dev->product_id,
                dev->serial
            );
        }

        count++;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return count;
}

#endif /* _WIN32 */

