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

#ifndef _UTILS_HPP_
#define _UTILS_HPP_

#include <cstdarg>
#include <string>
#include <stdint.h>

using namespace std;

#define MAX_CONVERT_BUFFER_SIZE  256

#define MAX_FORMAT_BUFFER_SIZE   4096
#define MAX_FORMAT_SIZE          MAX_FORMAT_BUFFER_SIZE / 2

class CUtils
{
public:
  // *******
  // Strings
  // *******

  static const string FormatString(const char* lpszFormat, va_list lpArgs);
  static const string FormatString(const char* lpszFormat, ...);

  // *******
  // Numbers
  // *******

  static string IntToString(int iValue);
  static string LongToString(long lValue);
  static string UnsignedIntToString(unsigned int uiValue);
  static string UnsignedLongToString(unsigned long ulValue);

  // ***********
  // File System
  // ***********
  static bool FileExists(const char* lpszFileName);
  static bool FileExists(const string& sFileName);    
  
  // ****
  // Math
  // ****
  
  static uint16_t CRC16(const unsigned char* lpData, size_t nSize);
};

#endif /* _UTILS_HPP_ */