FreeRTOS RINA library for microcontroller platforms
====

## 1. Overview
This piece of software is a RINA library implementation for FreeRTOS, 
so that RINA communication capabilities are supported by devices that 
use this RTOS (for information about RINA please visit 
http://pouzinsociety.org). Initially the RINA implementation can be used over the 
ESP32 hardware platform, using RINA over WiFi and RINA over Bluetooth.
This RINA library will be interoperable with IRATI, a RINA implementation
for Linux Operating Systems (https://github.com/IRATI/stack).

### 1.1 Platform support

#### FreeRTOS + ESP32

This code supports the ESP-IDF build system to build applications
directly for the ESP32

#### Arduino

Arduino support on ESP32 is based on Espress IDF platform but linking
the project as an Arduino projects opens the door to the code
eventually being used in the Arduino IDE with other Arduino
libraries. The code build for Arduino is roughly the same than what is
in the FreeRTOS + ESP32 port.

#### Linux

The Linux build supports receiving ethernet packets through a virtual
Tap interface created for that purpose. See README.Linux.md for
information how to use the Linux build.

## 2. What's working?

Not much! From the demo tree, beside all the effort spent on making
sure that the 3 targets work, a lot has been done to improve and
simplify the CDAP connection and the enrollment process. The code that
was written to support DTP packets remains present but has not seen
much love in the last weeks to month of development so it's likely
that trying to open an application flow is going to cause som
problem. Unlike the original prototype, this branch supports incoming
enrollment only. This means that the enrollment process has to be
triggered another host running IRATI.

A few memory leaks remain but they are few and far between and are not
the cause of immediate worry.
