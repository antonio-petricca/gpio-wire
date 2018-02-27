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

#ifndef _GPIO_WIRE_HPP_
#define _GPIO_WIRE_HPP_

#include <Utils.hpp>

// http://www.romanblack.com/RF/cheapRFmodules.htm

#define DEF_GPIO_ENCODER_STX '\x02'
#define DEF_GPIO_ENCODER_ETX '\x03'

class CGPIOWire
{
public:
  CGPIOWire(unsigned short uiDeviceNumber);
  
  bool Configure(
    unsigned long ulPinNumber,
    bool          bCanSleep,
    bool          bSwapOutput,
    unsigned long ulSyncBitCount,
    unsigned long ulHighStateEdge,
    unsigned long ulBitZeroDuration,
    unsigned long ulBitOneDuration,
    unsigned long ulBitSyncDuration,
    char          cSTX = DEF_GPIO_ENCODER_STX,
    char          cETX = DEF_GPIO_ENCODER_ETX
  );

  unsigned char* CreateMessage(
    const char* lpData, 
    size_t&     nSize, 
    bool        bCRC
  );

  unsigned char* CreateMessage(
    const string& sData, 
    size_t& nSize, 
    bool bCRC
  );
  
  static void FreeMessage(unsigned char* lpData);
  
  bool        Exists();
  
  bool        SendMessage(
    const unsigned char* lpMessage, 
    size_t nSize
  );

private:
  string         m_sDevice;
  unsigned short m_uiDeviceNumber;
  char           m_cETX;
  char           m_cSTX;
  
  bool SetParameter(
    const string& sSysClass, 
    const string& sName,
    const string& sValue
  );
  
  bool SetParameter(
    const string& sSysClass, 
    const string& sName,
    bool          bValue
  );
  
  inline bool SetParameter(
    const string& sSysClass, 
    const string& sName,
    unsigned int  uiValue
  );
  
  inline bool SetParameter(
    const string& sSysClass, 
    const string& sName,
    unsigned long ulValue
  );
};

#endif /* _GPIO_WIRE_HPP_ */