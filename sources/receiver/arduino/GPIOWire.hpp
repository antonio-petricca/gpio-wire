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

#ifndef _GPIOWire_H_
#define _GPIOWire_H_

namespace GPIOWire {

  // http://www.romanblack.com/RF/cheapRFmodules.htm

  #define DECODER_LOG_MASK_CONTROLLER      1
  #define DECODER_LOG_MASK_PULSE           2
  #define DECODER_LOG_MASK_SYNC            4
  #define DECODER_LOG_MASK_BIT             8
  #define DECODER_LOG_MASK_CHAR            16
  #define DECODER_LOG_MASK_BUFFER          32
  #define DECODER_LOG_MASK_NOISE           64
  #define DECODER_LOG_MASK_CRC_CALC        128
  #define DECODER_LOG_MASK_CRC_ERROR       256

  #define DECODER_LOG_LEVEL \
      DECODER_LOG_MASK_CRC_ERROR \
    + DECODER_LOG_MASK_CONTROLLER

  #define DECODER_BUFFER_SIZE              64
  #define DECODER_PULSES_DEBUG_BUFFER_SIZE ((8 + 2) * 3)

  typedef
    char GPIOWireBuffer[DECODER_BUFFER_SIZE];

  enum class MessageResult
  {
    Valid           = 0,
    NotYetAvailable = 1,
    BadCRC          = 2
  };

  void Initialize(
    unsigned int nInterruptPin,
    unsigned int nBitZero,
    unsigned int nBitOne,
    unsigned int nSyncBit,
    bool         bPullUp,
    char         cSTX,
    char         cETX
  );

  void          Start();
  void          Stop();

  bool          HasMessage();
  bool          IsStarted();

  MessageResult GetMessage(GPIOWireBuffer& lpMessage, bool bValidateCRC);
}

#endif /* _GPIOWire_H_ */
