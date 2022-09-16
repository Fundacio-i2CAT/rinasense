# Quick start

For the moment, the code can be built on Arduino *ONLY FOR ESP32
BOARDS*.

From within your Arduino IDE, install the ESP32 board package available here:

## Building

    $ git clone
    $ git submodule init
    $ git submodule update
    $ mkdir build && cd build
    $ cmake  ../CMakeLists.txt -DTARGET_TYPE=arduino_esp32 -DARDUINO_INSTALL_PATH=~/Programs/arduino-1.8.19 -DARDUINO_BOARD=esp32.firebeetle32
    $ make

## Uploading to the board

    $ sudo make upload SERIAL_PORT=/dev/ttyUSB0

## About ARDUINO\_BOARD

The _esp32.firebeetle32_ board ID only applies to the ESP32 FireBeetle
board that is used to develop this project. If you have a different
ESP32 board, and that you don't know its board ID, you can remove the
assignation to the variable ARDUINO\_BOARD\_ID variable from the CMake
command line above. When it runs, the Arduino CMake Toolchain
generates a big file in the _build_ directory called
_BoardOptions.cmake_ which you browse and where you'll find all the
boards supported by the board package. You can uncomment the line
corresponding to your board in that file and run CMake again the same
way to properly setup your sources for building.

# Arduino

The Arduino runtime on ESP32 is layered on top of the ESP runtime as
this project was originally developed. This runtime is exposed to the
Arduino applications that can use it as well as use more standard
Arduino libraries. This is why we could make this port with very
little changes to the source code.

This also means that this port currently working *only* on ESP32
boards. The reliance on POSIX features for portability to Linux and
the ESP32 network interface code will make it more difficult to build
a generic Arduino port. 

Anything that is specific to Arduino should obviously remain in the
*Portability* component. Right now, this is kept at a minimum and only
the logging routines in the component code calls the Arduino runtime.

## Short term goals

Right now the code can be built and uploade from the command
line. Eventually, we want the runtime packaged as a library that can
be used in the Arduino environment. Seeing the library is currently
building, it should only be a matter of properly packaging the build
artefacts along with a correct module description.

https://arduino.github.io/arduino-cli/0.27/library-specification/

## Long term goals

This port is currently strictly for ESP32 devices supported by the
Arduino IDE. For this code to run on a plain Arduino board that
doesn't, we would need to figure out how to support POSIX
threads. This is currently provided by the ESP32 runtime but also
supported by FreeRTOS+POSIX, which replicates the POSIX thread API
using FreeRTOS tasks. Seeing that there is at least [one version of
FreeRTOS](https://github.com/feilipu/Arduino_FreeRTOS_Library/tree/master/src)
that was built to work on top of the Arduino environment, it is not
unreasonable to think we could get FreeRTOS+POSIX running.


