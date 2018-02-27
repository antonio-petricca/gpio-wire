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

#include <stdio.h>
#include <string.h>

#include "GPIOWire.hpp"

#define GPIO_PIN_NUMBER        133
#define GPIO_CAN_SLEEP         false
#define GPIO_SWAP_OUTPUT       false

#define GPIO_SYNC_BIT_COUNT    5
#define GPIO_HIGH_STATE_EDGE   500
#define GPIO_BIT_ZERO_DURATION 1000
#define GPIO_BIT_ONE_DURATION  1500
#define GPIO_BIT_SYNC_DURATION 2500

#define GPIO_CRC               true

void Test_GPIOWire()
{
  #define MESSAGE "Hello from GPIO wire!"

  CGPIOWire GPIOWire(0);

  if (GPIOWire.Exists())
  {
    if (GPIOWire.Configure(
      GPIO_PIN_NUMBER,
      GPIO_CAN_SLEEP,
      GPIO_SWAP_OUTPUT,
      GPIO_SYNC_BIT_COUNT,
      GPIO_HIGH_STATE_EDGE,
      GPIO_BIT_ZERO_DURATION,
      GPIO_BIT_ONE_DURATION,
      GPIO_BIT_SYNC_DURATION
    ))
    {
      size_t         nSize     = strlen(MESSAGE);
      unsigned char* lpMessage = GPIOWire.CreateMessage(
        MESSAGE,
        nSize,
        GPIO_CRC 
      );

      GPIOWire.SendMessage(lpMessage, nSize);
      CGPIOWire::FreeMessage(lpMessage);
    }
  }
}

int main(int argc, char *argv[])
{
  Test_GPIOWire();

  return 0;
}
