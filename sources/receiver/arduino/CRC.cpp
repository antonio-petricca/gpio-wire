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

#include "CRC.hpp"
#include "Logger.hpp"

uint16_t CRC16(const unsigned char* lpData, size_t nSize)
{
  // https://stackoverflow.com/questions/10564491/function-to-calculate-a-crc16-checksum
  // https://www.lammertbies.nl/comm/info/crc-calculation.html

  // CRC-CCITT (0xFFFF)

  uint16_t nCRC = 0xFFFF;

  while (nSize--)
  {
    uint8_t nTemp  = (nCRC >> 8) ^ *lpData++;
    nTemp         ^= (nTemp >> 4);

    nCRC =
        (nCRC << 8)
      ^ ((unsigned short) (nTemp << 12))
      ^ ((unsigned short) (nTemp << 5))
      ^ ((unsigned short) nTemp)
    ;
  }

  return nCRC;
}
