/*

GPIO-Wire for Cheap RF communications
Copyright (C) 2016-2018 Antonio Petricca <antonio.petricca@gmail.com>

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

*/

#include "Logger.hpp"

// https://playground.arduino.cc/Main/CorruptArrayVariablesAndMemory
// https://www.arduino.cc/en/Reference/PROGMEM
// https://playground.arduino.cc/Main/Printf
// avr-objdump -s -j .data switch.elf | less

#ifdef _LOGGER_ENABLED

void SerialLogFmt(
  char*                      lpszBuffer,
  size_t                     nSize,
  const __FlashStringHelper *lpszFormat,
  ...
)
{
  va_list  lpArgs;

  va_start (lpArgs, lpszFormat);

#ifdef __AVR__
  // PROGMEM for AVR

  vsnprintf_P(
    lpszBuffer,
    nSize,
    (const char *)lpszFormat,
    lpArgs
  );
#else
  // For the rest of the world

  vsnprintf(
    buf,
    nSize,
    (const char *)fmt,
    args
  );
#endif

  va_end(lpArgs);

  Serial.println(lpszBuffer);
}

#endif
