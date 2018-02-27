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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "Utils.hpp"

uint16_t CUtils::CRC16(const unsigned char* lpData, size_t nSize)
{
  // https://stackoverflow.com/questions/10564491/function-to-calculate-a-crc16-checksum
  // https://www.lammertbies.nl/comm/info/crc-calculation.html

  // CRC-CCITT (0xFFFF)

  uint16_t nCRC = 0xFFFF;

  while (nSize--)
  {
      uint8_t nTemp  = (nCRC  >> 8) ^ *lpData++;
      nTemp         ^= (nTemp >> 4);

      nCRC   =
           (nCRC << 8)
        ^ ((unsigned short)(nTemp << 12))
        ^ ((unsigned short)(nTemp <<  5))
        ^ ((unsigned short)nTemp)
      ;
  }

  return nCRC;
}

bool CUtils::FileExists(const char* lpszFileName)
{
  assert(lpszFileName);

  return (-1 != access(lpszFileName, F_OK));
}

bool CUtils::FileExists(const string& sFileName)
{
  return FileExists(sFileName.c_str());
}

const string CUtils::FormatString(const char* lpszFormat, va_list lpArgs)
{
  assert(lpszFormat);

  int iLength = strlen(lpszFormat);

  if (iLength > MAX_FORMAT_SIZE)
  {
    return string(lpszFormat);
  }

  char lpszMessage[MAX_FORMAT_BUFFER_SIZE + 1];

  iLength = vsnprintf(
    lpszMessage,
    (MAX_FORMAT_BUFFER_SIZE + 1),
    lpszFormat,
    lpArgs
  );

  if (iLength >= MAX_FORMAT_BUFFER_SIZE)
  {
    return string(lpszFormat);
  }

  return string(lpszMessage);
}

const string CUtils::FormatString(const char* lpszFormat, ...)
{
  va_list lpArgs;
  va_start(lpArgs, lpszFormat);
  string sReturn = FormatString(lpszFormat, lpArgs);
  va_end(lpArgs);

  return sReturn;
}
string CUtils::IntToString(int iValue)
{
  return LongToString(iValue);
}

string CUtils::LongToString(long lValue)
{
  char szBuffer[MAX_CONVERT_BUFFER_SIZE];

  snprintf(szBuffer, sizeof(szBuffer), "%ld", lValue);
  return string(szBuffer);
}

string CUtils::UnsignedIntToString(unsigned int uiValue)
{
  return UnsignedLongToString(uiValue);
}

string CUtils::UnsignedLongToString(unsigned long ulValue)
{
  char szBuffer[MAX_CONVERT_BUFFER_SIZE];

  snprintf(szBuffer, sizeof(szBuffer), "%lu", ulValue);
  return string(szBuffer);
}