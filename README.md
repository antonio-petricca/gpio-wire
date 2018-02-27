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
using cheap RF Tx/Rx 433/315 Mhz, but I cannot get any which can transmit data
from a linux board and receive on Arduino using a reliable protocol, supporting
a very precise timing (HR timers).

About HR timers I have published a test module available at:

  https://github.com/DareDevil73/hr-timers-tester

As stated early, I have based my work on the assumptions and protocol designed
by Roman Black.

In the beginning I wrote code to address C.H.I.P. board (https://getchip.com/),
but it can be easily ported to other embedded platforms Raspberry like.

------------
Project tree
------------

+ documents (Roman Black study)
|
- sources :
  |
  +- receiver (Rx software)
  |  |
  |  +- arduino (Arduino Rx C++  implementation)
  |
  +- transmitter (Tx software)
     |
     +- library (Client library classes)
     |
     +- module (Linux kernel module)
     |
     +- scripts (C.H.I.P. building scrits)
     |
     +- tester (Tx C++ demo application)

-----------------------
Pre-Requisites: Arduino
-----------------------

The Arduino project is a pure Arduino C++, so I suggest you to use the Sloeber
IDE (Eclipse based) available at:

  http://eclipse.baeyens.it/

------------------------
Pre-Requisites: C.H.I.P.
------------------------

Basically you need only "cmake" and "g++ 5".

---------------------
Pre-Requisites: Linux
---------------------

Basically you need only "cmake" and "g++ 5", but in order to use GPIO mock-up
kernel module you need a kernel 4.10 or above.

Please look at these links:

 - https://github.com/torvalds/linux/blob/v4.10/tools/testing/selftests/gpio/gpio-mockup.sh
 - https://github.com/torvalds/linux/blob/master/drivers/gpio/gpio-mockup.c

--------------
Build: Arduino
--------------

- Import Arduino project into Sloeber:

  - Create new Arduino sketch.
  - Give it any name you want (e.g. "deleteme").
  - Set as workspace folder the Rx folder.
  - Setup Arduino hardware parameters.
  - Set "Default cpp file" project type.
  - Remove default files (e.g. "deleteme.*").
  - Try building all (CTRL+B).

- Customize "Configuration.hpp" timing parameters to match the "tester" ones.
- Build and Upload.

--------------------------
Build: Linux kernel module
--------------------------

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
   disable CRC check on Rx) to debug without have to compile tester Tx.

So, for example, to build the kernel module on your preferred linux distribution
you could you issue these commands:

- make build
- make install-mockup
- make log-show

On your linux distribution you may not have then "gpio-mock.ko", so you have to
download kernel sources and build it on your own following the standard kernel
compiling process.

----------------
Build: Tx Tester
----------------

- ./clean
- ./configure
- ./build

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
