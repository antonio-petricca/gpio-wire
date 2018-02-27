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
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "GPIOWire.hpp"

CGPIOWire::CGPIOWire(unsigned short uiDeviceNumber)
  : m_sDevice(CUtils::FormatString("/dev/gpiowire%d", uiDeviceNumber))
  , m_uiDeviceNumber(uiDeviceNumber)
{
  assert(uiDeviceNumber >= 0);
}

bool CGPIOWire::Configure(
  unsigned long  ulPinNumber,
  bool           bCanSleep,
  bool           bSwapOutput,
  unsigned long  ulSyncBitCount,
  unsigned long  ulHighStateEdge,
  unsigned long  ulBitZeroDuration,
  unsigned long  ulBitOneDuration,
  unsigned long  ulBitSyncDuration,
  char           cSTX,
  char           cETX
)
{
  m_cETX = cETX;
  m_cSTX = cSTX;

  string sSysClass(CUtils::FormatString(
    "/sys/class/gpiowires/gpiowire%d",
    m_uiDeviceNumber)
  );

  return
       SetParameter(sSysClass, "pinNumber",       ulPinNumber)
    && SetParameter(sSysClass, "canSleep",        bCanSleep)
    && SetParameter(sSysClass, "swapOutput",      bSwapOutput)
    && SetParameter(sSysClass, "bitSyncCount",    ulSyncBitCount)
    && SetParameter(sSysClass, "highStateEdge",   ulHighStateEdge)
    && SetParameter(sSysClass, "bitZeroDuration", ulBitZeroDuration)
    && SetParameter(sSysClass, "bitOneDuration",  ulBitOneDuration)
    && SetParameter(sSysClass, "bitSyncDuration", ulBitSyncDuration)
  ;
}

unsigned char* CGPIOWire::CreateMessage(
  const char* lpData,
  size_t&     nSize,
  bool        bCRC
)
{
  size_t nBufferSize = (nSize + 2); // STX + ETX

  if (bCRC)
  {
    nBufferSize += 2;
  }

  unsigned char* lpBuffer =
    (unsigned char *)calloc(sizeof(char), nBufferSize);

  lpBuffer[0]               = m_cSTX;
  lpBuffer[nBufferSize - 1] = m_cETX;

  if (bCRC)
  {
    uint16_t nCRC = CUtils::CRC16(
      (unsigned char *)lpData,
      nSize
    );

    lpBuffer[nBufferSize - 2] = (nCRC & 0xFF);
    lpBuffer[nBufferSize - 3] = ((nCRC >> 8) & 0xFF);
  }

  memcpy(
    (lpBuffer + sizeof(char)),
    lpData,
    nSize
  );

  nSize = nBufferSize;

  return lpBuffer;
}

unsigned char* CGPIOWire::CreateMessage(
  const string& sData,
  size_t&       nSize,
  bool          bCRC
)
{
  nSize = sData.length();

  return CreateMessage(sData.c_str(), nSize, bCRC);
}

bool CGPIOWire::Exists()
{
  return CUtils::FileExists(m_sDevice);
}

void CGPIOWire::FreeMessage(unsigned char* lpMessage)
{
  assert(lpMessage);

  if (lpMessage)
  {
    free(lpMessage);
  }
}

bool CGPIOWire::SendMessage(const unsigned char* lpMessage, size_t nSize)
{
  int iHandle = open(m_sDevice.c_str(), O_WRONLY);

  if (-1 == iHandle)
  {
    return false;
  }

  int iBytesWritten = write(iHandle, lpMessage, nSize);

  if (-1 == iBytesWritten)
  {
    close(iHandle);

    return false;
  }

  close(iHandle);

  return true;
}

bool CGPIOWire::SetParameter(
  const string& sSysClass,
  const string& sName,
  const string& sValue
)
{
  string sPath = CUtils::FormatString(
    "%s/settings/%s",
    sSysClass.c_str(),
    sName.c_str()
  );

  int iHandle = open(sPath.c_str(), O_WRONLY);

  if (-1 == iHandle)
  {
    return false;
  }

  int iBytesWritten = write(iHandle, sValue.c_str(), sValue.length());

  if (-1 == iBytesWritten)
  {
    close(iHandle);

    return false;
  }

  close(iHandle);

  return true;
}

bool CGPIOWire::SetParameter(
  const string& sSysClass,
  const string& sName,
  bool          bValue
)
{
  return SetParameter(
    sSysClass,
    sName,
    CUtils::IntToString(bValue ? 1 : 0)
  );
}

bool CGPIOWire::SetParameter(
  const string& sSysClass,
  const string& sName,
  unsigned int  uiValue
)
{
  return SetParameter(
    sSysClass,
    sName,
    CUtils::UnsignedIntToString(uiValue)
  );
}

bool CGPIOWire::SetParameter(
  const string& sSysClass,
  const string& sName,
  unsigned long ulValue
)
{
  return SetParameter(
    sSysClass,
    sName,
    CUtils::UnsignedLongToString(ulValue)
  );
}