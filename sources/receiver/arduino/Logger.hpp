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

#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#define _LOGGER_ENABLED

#include "Arduino.h"

#ifdef _LOGGER_ENABLED

	#define LOG_BUF_SIZE 128 /* Keep it low to avoid stack overflow/corruption! */

	#define LOG_INITIALIZE() \
		Serial.begin(115200)

  #define LOG_DECLARE_BUFFER() \
    char szLogBuffer[LOG_BUF_SIZE]

  #define LOG(sMessage) \
    Serial.println(F(sMessage))

  #define LOG_FMT(sFormat, ...) \
    SerialLogFmt(szLogBuffer, sizeof(szLogBuffer), F(sFormat), ##__VA_ARGS__);

  void SerialLogFmt(
    char*                      lpszBuffer,
    size_t                     nSize,
    const __FlashStringHelper *lpszFormat,
                               ...
  );

#else

	#define LOG_INITIALIZE()
  #define LOG_DECLARE_BUFFER()

	#define LOG(sMessage)
	#define LOG_FMT(sFormat, ...)

#endif

#endif /* _LOGGER_HPP_ */
