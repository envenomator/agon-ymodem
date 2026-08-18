# Agon YMODEM send/receive utilities
The purpose of these utilities is to
provide an easy mechanism to send/receive files to/from your Agon filesystem, using the VDP's USB-Serial port.

# Installation
Copy 'ymodem.bin' to the /bin/ directory on the microSD card. If no such directory exists, create it first using the 'mkdir' command.

The utility requires at least MOS v2.2.0 and VDP firmware v2.16.0 or newer (which includes built-in YMODEM support). For instructions on updating your firmware, see the [Agon Updating Firmware documentation](https://agonplatform.github.io/agon-docs/Updating-Firmware/).

# Usage
## Agon ymodem utility
```
    Usage:
      ymodem -r [directory]       Receive mode, optional target directory
      ymodem -s file1 [file2 ...] Send mode, at least one file required
```

### Sending files from Agon
On the Agon side:
```
ymodem -s file1 [file2 ...]
```
On the PC side:
```
ymodem -r [directory]
```
Sending an entire directory is still a work-in-progress.

### Receiving files to Agon
On the Agon side:
```
ymodem -r [directory]
```
On the PC side:
```
ymodem -s file1 [file2 ...]
```

# Serial connectivity
Connect the VDP USB port to your PC and find the name of it's serial device. This may be /dev/ttyUSB0 under Linux, /dev/cu.usbserialXXX under MacOS and COMXXX under Windows.

The compiled-in VDP serial baudrate is 115200, 8 bits, 1 stopbit, no parity (8N1)

# PC utility examples
## YMODEM utility
The ymodem utility can be downloaded from the release folder. It tries to autodetect the USB-serial interface. If multiple such interfaces are present on the system, it lists them and exits the program. A specific device can be selected using the '-d' flag.

## LRZSZ
This example assumes the usage of a /dev/ttyUSB0 device. Your setup will likely be different.
The 'lrzsz' package may be used, using 'rz' for receiving and 'sz' for sending files to/from your PC. The package does not provide a way to directly talk to the serial port, not set the baudrate, so that has to be done using redirections and using the stty command. 

### Sending files to your Agon
Sending all files from a 'test' directory, using 1K blocks:
```
stty -F /dev/ttyUSB0 115200 & sz -k --ymodem ./test/* 1>/dev/ttyUSB0 0</dev/ttyUSB0
```
**MacOS users should use stty -f instead of -F**

Start the MOS utility, optionally give it a directory name. The transfer starts as soon as the utility starts.
```
ymodem -r [directory]
```
### Receiving files to your PC
Start the MOS utility, provide it with all files to send to your PC:
```
ymodem -s file1 [file2 ...]
```
Receiving all files to the current directory. The transfer starts as soon as the rz utility starts:
```
stty -F /dev/ttyUSB0 115200 & rz --overwrite --ymodem 1>/dev/ttyUSB0 0</dev/ttyUSB0
```
**MacOS users should use stty -f instead of -F**

## Python YMODEM module
This example assumes the usage of a /dev/ttyUSB0 device. Your setup will likely be different.
### Setup
Using macOS or Linux, I usually create a python virtual environment for this using these steps
```bash
python3 -m venv venv
source venv/bin/activate
pip install ymodem
```

and then use
```bash
deactivate
```
when I'm done receiving/sending files

### Sending files to your Agon
Sending all files from 'test' directory:
```bash
python -m ymodem send ./test/* -p /dev/ttyUSB0 -b 115200
```
Start the MOS utility, optionally give it a directory name. The transfer starts as soon as the utility starts:
```
ymodem -r [directory]
```
### Receiving files to your PC
Start the MOS utility, provide it with all files to send to your PC:
```
ymodem -s file1 [file2 ...]
```
Start the ymodem receive function, storing all files to your local directory:
```bash
python -m ymodem recv -p /dev/cu.usbserial-02B1CCC5 -b 115200 .
```

## tio (macOS / Linux)
[tio](https://github.com/tio/tio) is a simple and feature-rich serial terminal that supports YMODEM file transfers directly.

> **Note**: `tio`'s built-in YMODEM support (`ctrl-t y`) is only used to **send** files from your computer to the Agon Light. It does not support receiving files from the Agon Light via YMODEM. To receive files on your computer, use one of the other methods (such as `python -m ymodem recv`, the compiled C/C++ `ymodem` PC tool, or `rz`).

### Installation (macOS with Homebrew)
```bash
brew install tio
```

### Finding your serial device & connecting
List connected serial devices:
```bash
tio -l
```
Connect to the Agon (default baudrate is 115200):
```bash
tio /dev/cu.usbserialXXX
```
> **Tip**: To exit `tio` at any time, press `ctrl-t` followed by `q`. You can also press `ctrl-t ?` to view all available session key commands.

### Sending files to your Agon
1. On the Agon, start receive mode:
   ```
   ymodem -r [directory]
   ```
2. Connect to the Agon using `tio` (or within an existing session):
   ```bash
   tio /dev/cu.usbserialXXX
   ```
3. Press `ctrl-t` followed by `y` to initiate a YMODEM send.
4. Enter the path of the file to send at the prompt.
5. Exit `tio` when finished using `ctrl-t` followed by `q`.
