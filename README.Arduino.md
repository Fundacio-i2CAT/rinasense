# Quick start

For the moment, the code can be built on Arduino *ONLY FOR ESP32
BOARDS*.

From within your Arduino IDE, install the ESP32 board package available here:

https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

The documentation for this is available here:

https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/installing.html

## Building

To build this project from Arduino from a fresh clone, you need to do
the following steps:

    $ git clone [RinaSense repository URL]
    $ git submodule init
    $ git submodule update
    $ mkdir build && cd build
    $ cmake  ../CMakeLists.txt -DTARGET_TYPE=arduino_esp32 -DARDUINO_INSTALL_PATH=~/Programs/arduino-1.8.19 -DARDUINO_BOARD=esp32.firebeetle32
    $ make

Out of the 3 variables set on the command line, only 2 are
required. _TARGET\_TYPE_ is used in the CMake files to determine which
kind of output of the project. _ARDUINO\_INSTALL_PATH_ is the
directory where your Arduino IDE files are installed. See below for
the meaning of the _ARDUINO\_BOARD_ variable.

### About ARDUINO\_BOARD

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

## Uploading to the board

This generally requires _root_ access because ordinary users don't
tend to have access to serial ports.

    $ sudo make upload SERIAL_PORT=/dev/ttyUSB0

# Arduino

The Arduino runtime on ESP32 is layered on top of the ESP
runtime. This project was originally developed the same way but was
changed to be more portable. This runtime is exposed to the Arduino
applications so that it can use it as well as use more standard, or
easier to use, Arduino libraries. This is why we could make this port
with very little changes to the source code.

Unfortunately, this also means that this port currently working *only*
on ESP32 boards. The usage of the ESP32 wifi library and the reliance
on POSIX features for portability to Linux and the ESP32 network
interface code will make it more difficult to build a generic Arduino
port. The underlying ESP runtime provides some POSIX features, such as
pthread, that are not present on ordinary Arduino board runtime.

RinaSense aims to be portable to more platforms than Arduino. This
means that anything that is specific to Arduino should be coded within
the *Portability* component. Right now, only the logging routines code
calls into the Arduino runtime.

## Short term goals

Right now the code can be built and uploaded from the command
line. Eventually, we want the runtime packaged as a library that can
be used in the Arduino environment. Seeing the library is currently
building, it should only be a matter of properly packaging the build
artefacts along with a correct module description.

https://arduino.github.io/arduino-cli/0.27/library-specification/

