---------
Licensing
---------

GPIO-Wire for Cheap RF communications
Copyright (C) 2016-2018  Antonio Petricca <antonio.petricca@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

------------
Introduction
------------

This project comes from an IoT project of mine, after a lot of research,
experimentation and coding.

I have used many open source studies as knowledge base, so I would share with
the community my work.

I would especially thank a guy, Mr Roman Black and his amazing study on cheap RF
modules (like FS1000A):

  http://www.romanblack.com/RF/cheapRFmodules.htm

-------------
Project scope
-------------

Internet is full of open source projects which can transmit and receive data
using cheap RF TX/RX 433/315 Mhz, but I cannot get any which can transmit data
from a linux board (non real time OS) and receive on Arduino using a reliable
protocol, based upon a very precise timing (by HR timers).

About HR timers I have published a test module available at:

  https://github.com/DareDevil73/hr-timers-tester

As stated early, I have based my work on the assumptions and protocol designed
by Roman Black which test and implement a pulse with modulation binary protocol.

In the beginning I wrote code to address C.H.I.P. board (https://getchip.com/),
but it can be easily ported to other embedded platforms Raspberry like.

I have improved the basic protocol by writing a linux kernel module:

 - available as chacter device, so writable like a file;
 - based on High Resolution Timers to get precise RF edges timing (not
   achievable by other sleeping techinques);
 - fully customizable in terms of:
   - number of virtual devices, one for each needed GPIO physical pin : set by
     the "devicesNumber" module initialization parameter;
   - protocol behaviour :
     - "pinNumber"       : the GPIO physical pin number;
     - "bitSyncCount"    : number of data byte synchronization bits count;
     - "highStateEdge"   : high state edge duration (uS);
     - "bitZeroDuration" : bit 0 duration (uS);
     - "bitOneDuration"  : bit 1 duration (uS);
     - "bitSyncDuration" : synchronization bit duration (uS).
     - "swapOutput"      : invert/revert GPIO pin logic (some TX models need
        different signal triggering edge);
     - "perfDebug"       : write to kernel log edge performance information,
       useful to debug timing issues;
 - added an optional CRC16 (CRC-CCITT) to ensure message correctness;

This inequality must be satisfied:

  highStateEdge < bitZeroDuration < bitOneDuration < bitSyncDuration

The kernel module has been wrapped by a client class (CGPIOWire).

On the receiver side I have added an automatic noise threshold to exclude false
positives for very noisy RF devices.

------------------------
Project tree explanation
------------------------

- documents                   : Roman Black study.
- sources/receiver/arduino    : Arduino RX C++ implementation.
- sources/transmitter/library : Client library classes.
- sources/transmitter/module  : Linux kernel module.
- sources/transmitter/scripts : C.H.I.P. building scripts.
- sources/transmitter/tester  : TX C++ demo application.

---------------------
Pre-Requisites: Linux
---------------------

Basically you need only "cmake" and "g++ 5", but in order to use GPIO mock-up
kernel module you need a kernel 4.10 or above.

Please look at these links:

 - https://github.com/torvalds/linux/blob/v4.10/tools/testing/selftests/gpio/gpio-mockup.sh
 - https://github.com/torvalds/linux/blob/master/drivers/gpio/gpio-mockup.c

----------------------------
Pre-Requisites: Arduino (RX)
----------------------------

The Arduino project is a pure Arduino C++, so I suggest you to use the Sloeber
IDE (Eclipse based) available at:

  http://eclipse.baeyens.it/

------------------------
Pre-Requisites: C.H.I.P.
------------------------

Basically you need only "cmake" and "g++ 5".

-------------------------------
Build: Linux kernel module (TX)
-------------------------------

By the "make [command-id]" command you can issue these commands:

 - "all", "build" : build on generic linux distribution.

 - "chip-install-sources": download kernel sources for C.H.I.P platform from
   https://github.com/NextThingCo/CHIP-linux/archive/debian/CHIP-linux-debian-4.4.13-ntc-mlc.zip .

 - "chip-build": build on C.H.I.P. platform.

 - "clean": remove any building intermediate file.

 - "gdb": debug kernel oops (look at comments inside Makefile).

 - "get-pin-number": print currently configured GPIO PIN number.

 - "install": load kernel module on generic linux distribution using the real
   board hardware.

 - "install-mockup": load kernel module on generic linux distribution using
   the kernel GPIO mock-up driver in place of the real hardware.

 - "ls-mod": check for a GPIO loaded kernel modules.

 - "log-show": print kernel log.

 - "log-tail": tail kernel log.

 - "mod-info": print kernel module information (if loaded by "install*").

 - "mod-probe": probe kernel module information (if loaded by "install*").

 - "mod-rm-mockup": unload the GPIO mock-up driver (if loaded).

 - "set-perf-debug-off", "set-perf-debug-on": (de)activate logging of HR timers
   performance/precision debugging information (look at kernel log).

 - "set-pin-number-1", "set-pin-number-2" : set GPIO pin to a value set by
   "pin-number-1" or "pin-number-2" variables.

 - "set-pin-swap-off", "set-pin-swap-on" : invert/revert GPIO pin signal logic.

 - "set-log-debug-level" : increase kernel log verbosity.

 - "setup-default", "setup-fast", "setup-medium", "setup-slow" : configure some
   timing profiles (to be matched with receiver setup).

 - "uninstall" : unload kernel module (real hardware).

 - "uninstall-mockup" : unload mock-up and kernel module.

 - "write-no-crc" : ask a string to send to receiver without CRC (remember to
   disable CRC check on RX) to debug without have to compile tester TX.

So, for example, to build the kernel module on your preferred linux distribution
you could you issue these commands:

- make build
- make install-mockup
- make log-show

On your linux distribution you may not have then "gpio-mock.ko", so you have to
download kernel sources and build it on your own following the standard kernel
compiling process.

------------------
Build: Tester (TX)
------------------

- ./clean
- ./configure
- ./build

-------------------
Build: Arduino (RX)
-------------------

- Import Arduino project into Sloeber:

  - Create new Arduino sketch.
  - Give it any name you want (e.g. "deleteme").
  - Set as workspace folder the RX folder.
  - Setup Arduino hardware parameters.
  - Set "Default cpp file" project type.
  - Remove default files (e.g. "deleteme.*").
  - Try building all (CTRL+B).

- Customize "Configuration.hpp" timing parameters to match the "tester" ones.
- Build and Upload.

-------
Support
-------

Please use GitHub issue tracker.

------------
Known issues
------------

I have extracted and organized my source code from a real hardware project.

In the that scenario it works perfectly.

Unfortunately I have no time to test all the modules again so, please, inform me
about any issue you may incur in.

Thank you,
Antonio
