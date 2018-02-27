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

#ifndef _CONFIGURATION_HPP_
#define _CONFIGURATION_HPP_

#define PIN_INTERRUPT        2

#define DECODER_BIT_ZERO     1000
#define DECODER_BIT_ONE      1500
#define DECODER_BIT_SYNC     2500

#define DECODER_PULLUP       false
#define DECODER_STX          '\x02'
#define DECODER_ETX          '\x03'
#define DECODER_VALIDATE_CRC true

#define LED_BLINK_TIMEOUT    500 * 1000 /* Microseconds */
#define LED_FLASH_TIMEOUT    125        /* Milliseconds */
#define LED_FLASH_COUNT      3

/*
 * Configure logging levels on:
 *
 *  - GPIOWire.hpp
 */

#endif /* _CONFIGURATION_HPP_ */
