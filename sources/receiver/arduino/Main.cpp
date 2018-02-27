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

#include "Main.hpp"

void setup()
{
  LOG_INITIALIZE();
  LOG_DECLARE_BUFFER();

  GPIOWire::Initialize(
    PIN_INTERRUPT,
    DECODER_BIT_ZERO,
    DECODER_BIT_ONE,
    DECODER_BIT_SYNC,
    DECODER_PULLUP,
    DECODER_STX,
    DECODER_ETX
  );

  GPIOWire::Start();
}

void loop()
{
  LOG_INITIALIZE();
  LOG_DECLARE_BUFFER();

  GPIOWire::GPIOWireBuffer lpMessage;

  switch (GPIOWire::GetMessage(lpMessage, DECODER_VALIDATE_CRC))
  {
    case GPIOWire::MessageResult::Valid :
	  LOG_FMT("Got message \"%s\".", lpMessage);
      GPIOWire::Start();

      break;

    case GPIOWire::MessageResult::BadCRC :
      LOG("Bad CRC!");
      GPIOWire::Start();

      break;

    case GPIOWire::MessageResult::NotYetAvailable :
      // Do nothing so far...
      break;
  }
}
