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

#include "Arduino.h"

#include "CRC.hpp"
#include "GPIOWire.hpp"
#include "Logger.hpp"

namespace GPIOWire {

  /***********/
  /* Private */
  /***********/

  enum class BufferStatus
  {
    WaitingForSTX  = 0,
    WaitingForETX  = 1,
    MessageDecoded = 2,
    MessageGot     = 3,
    Overflow       = 4
  };

  enum class DecoderStatus
  {
    WaitingForSync = 0,
    WaitingForData = 1,
    Decoding       = 2
  };

  bool                   m_bStarted = false;

  unsigned int           m_nBitMidPoint;
  unsigned int           m_nBitLowPoint;
  unsigned int           m_nSyncMidPoint;
  volatile unsigned long m_nLastSampling;

  volatile uint8_t       m_nDecodedBits;
  volatile unsigned int  m_nDecodedBytes;
  volatile DecoderStatus m_eDecoderStatus;
  volatile uint8_t       m_nDecodingData;

  GPIOWireBuffer         m_lpBuffer;
  volatile unsigned int  m_nBufferIndex;
  unsigned int           m_nBufferSize;
  volatile BufferStatus  m_eBufferStatus;
  char                   m_cSTX;
  char                   m_cETX;

  LOG_DECLARE_BUFFER();

  inline void ProcessDecodedData(char cData)
  {
    switch (m_eBufferStatus)
    {
      case BufferStatus::WaitingForSTX :
        if (cData == m_cSTX)
        {
          m_eBufferStatus = BufferStatus::WaitingForETX;

          #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_BUFFER
            LOG("Wire: STX.");
          #endif
        }

        m_nBufferIndex = 0;
        break;

      case BufferStatus::WaitingForETX :
        if (cData == m_cETX)
        {
          Stop();
          m_eBufferStatus = BufferStatus::MessageDecoded;

          #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_BUFFER
            LOG("Wire: ETX.");
          #endif
        }
        else if (m_nBufferIndex >= (m_nBufferSize - 1))
        {
          Stop();
          m_eBufferStatus = BufferStatus::Overflow;

          #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_BUFFER
            LOG("GPIOWire: buffer overflow!");
          #endif
        }
        else
        {
          m_lpBuffer[m_nBufferIndex] = cData;
          m_nBufferIndex++;

          if (m_nBufferIndex < (m_nBufferSize - 1))
          {
            #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_BUFFER
              LOG_FMT(
                "Wire: char added (size %d).",
                m_nBufferIndex
              );
            #endif
          }
          else
          {
            Stop();
            m_eBufferStatus = BufferStatus::MessageDecoded;

            #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_BUFFER
              LOG("Wire: char appended (full).");
            #endif
          }
        }

        break;

      default:
        // Avoid Sloeber / Eclipse warnings for not handled cases.

        break;
    }
  }

  void OnInterrupt()
  {
    if (m_bStarted)
    {
      // Capture time slice

      unsigned long nCurrentTime   = micros();
      unsigned long nPulseDuration = nCurrentTime - m_nLastSampling;

      #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_PULSE
        LOG_FMT("Wire: pulse %lu uS.", nPulseDuration);
      #endif

      m_nLastSampling = nCurrentTime;

      // Decoding...

      if (nPulseDuration >= m_nSyncMidPoint)
      {
        // Received Sync pulse

        #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_SYNC
          LOG_FMT("Wire: sync (%lu uS).", nPulseDuration);
        #endif

        m_eDecoderStatus = DecoderStatus::WaitingForData;
        m_nDecodedBits   = 0;
        m_nDecodingData  = 0;
      }
      else if (nPulseDuration < m_nBitLowPoint)
      {
        // Received noise pulse

        #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_NOISE
          LOG_FMT(
            "Wire: noise (%lu uS).",
            nPulseDuration
          );
        #endif

        m_eDecoderStatus = DecoderStatus::WaitingForSync;
        m_nDecodedBits   = 0;
        m_nDecodingData  = 0;
      }
      else
      {
        // Decode bits

        if (m_nDecodedBits < 8)
        {
          // Decode bit

          m_nDecodedBits++;

          m_eDecoderStatus        = DecoderStatus::Decoding;
          unsigned char nBitValue = ((nPulseDuration < m_nBitMidPoint) ? 0 : 1);

          #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_BIT
            LOG_FMT(
              "Wire: bit #%i = %d (%lu uS).",
              m_nDecodedBits,
              nBitValue,
              nPulseDuration
            );
          #endif

          m_nDecodingData <<= 1;
          m_nDecodingData  |= nBitValue;
        }

        if (8 == m_nDecodedBits)
        {
          // Store completed byte

          #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_CHAR
            LOG_FMT(
              "Wire: byte %d (0x%02X) '%c'.",
              m_nDecodingData,
              m_nDecodingData,
              (char)m_nDecodingData
            );
          #endif

          ProcessDecodedData(m_nDecodingData);

          m_eDecoderStatus = DecoderStatus::WaitingForSync;
          m_nDecodedBits   = 0;
          m_nDecodingData  = 0;
        }
      }

    }
  }

  bool ValidateCRC()
  {
    if (m_nBufferIndex < 2)
    {
      #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_CRC_ERROR
        LOG("Wire: bad CRC, invalid buffer size.");
      #endif

      return false;
    }

    uint16_t nCalculatedCRC = CRC16(
      (const unsigned char*)m_lpBuffer,
      (m_nBufferIndex - 2)
    );

    uint16_t nPassedCRC =
        ((unsigned char)m_lpBuffer[m_nBufferIndex - 2] << 8)
      |  (unsigned char)m_lpBuffer[m_nBufferIndex - 1]
    ;

    #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_CRC_CALC
      LOG_FMT("Wire: CRC (passed = 0x%04X, calc = 0x%04X).",
        nPassedCRC,
        nCalculatedCRC
      );
    #endif

    bool bValid = (nPassedCRC == nCalculatedCRC);

    #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_CRC_ERROR
      if (!bValid)
      {
        LOG_FMT(
          "Wire: bad CRC (passed = 0x%04X, calc = 0x%04X).",
          nPassedCRC,
          nCalculatedCRC
        );
      };
    #endif

    if (bValid)
    {
      m_nBufferIndex -= 2;
    }

    return bValid;
  }

  /**********/
  /* Public */
  /**********/

  MessageResult GetMessage(GPIOWireBuffer& lpMessage, bool bValidateCRC)
  {
    if (BufferStatus::MessageDecoded != m_eBufferStatus)
    {
      return MessageResult::NotYetAvailable;
    }

    m_eBufferStatus = BufferStatus::MessageGot;

    if (bValidateCRC && !ValidateCRC())
    {
      return MessageResult::BadCRC;
    }

    memcpy(lpMessage, m_lpBuffer, m_nBufferIndex);
    lpMessage[m_nBufferIndex] = '\0';

    return MessageResult::Valid;
  }

  bool HasMessage()
  {
    return (BufferStatus::MessageDecoded == m_eBufferStatus);
  }

  bool IsStarted()
  {
    return m_bStarted;
  }

  void Initialize(
    unsigned int nInterruptPin,
    unsigned int nBitZero,
    unsigned int nBitOne,
    unsigned int nSyncBit,
    bool         bPullUp,
    char         cSTX,
    char         cETX
  )
  {
    assert(nBitZero > 0);
    assert(nBitOne  > nBitZero);
    assert(nSyncBit > nBitOne);

    m_cSTX           = cSTX;
    m_cETX           = cETX;

    m_nBitMidPoint   = ((nBitOne + nBitZero) / 2);
    m_nBitLowPoint   = (nBitZero - (m_nBitMidPoint - nBitZero));
    m_nSyncMidPoint  = ((nSyncBit + nBitOne) / 2);

    #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_CONTROLLER
      LOG_FMT("Wire: bit 0 = %lu us.", nBitZero);
      LOG_FMT("Wire: bit 1 = %lu us.", nBitOne);
      LOG_FMT("Wire: bit S = %lu us.", nSyncBit);

      LOG_FMT("Wire: bit low pt  = %lu us.", m_nBitLowPoint);
      LOG_FMT("Wire: bit mid pt  = %lu us.", m_nBitMidPoint);
      LOG_FMT("Wire: sync mid pt = %lu us.", m_nSyncMidPoint);
    #endif

    Stop();

    pinMode(
      nInterruptPin,
      (bPullUp ? INPUT_PULLUP : INPUT)
    );

    attachInterrupt(
      digitalPinToInterrupt(nInterruptPin),
      OnInterrupt,
      FALLING
    );
  }

  void Start()
  {
    Stop();

    m_nDecodingData  = 0;
    m_nDecodedBits   = 0,
    m_nDecodedBytes  = 0;
    m_eDecoderStatus = DecoderStatus::WaitingForSync;

    m_nBufferIndex   = 0;
    m_nBufferSize    = (DECODER_BUFFER_SIZE - 1 /* Leave room for \0 */);
    m_eBufferStatus  = BufferStatus::WaitingForSTX;

    m_nLastSampling  = micros();
    m_bStarted       = true;

    #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_CONTROLLER
      LOG("Wire: started.");
    #endif
  }

  void Stop()
  {
    if (m_bStarted)
    {
      m_bStarted = false;

      #if DECODER_LOG_LEVEL & DECODER_LOG_MASK_CONTROLLER
        LOG("Wire: stopped.");
      #endif
    }
  }

}
