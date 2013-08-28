/*
  twi.h - TWI/I2C library for Wiring & Arduino
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef twi_h
#define twi_h

  #include <inttypes.h>

  //#define ATMEGA8

  #ifndef TWI_FREQ
  #define TWI_FREQ 100000L
  #endif

  #define TWI_READY 0
  #define TWI_MRX   1
  #define TWI_MTX   2
  
  #define TWI_COMPLETE    0
  #define TWI_PENDING     1
  #define TWI_AWAIT_STOP  2
    
  void twi_init(void);

  void twi_reply(uint8_t);
  void twi_stop(void);
  void twi_releaseBus(void);
  
  uint8_t twi_asyncWriteRead(uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t, uint8_t);
  uint8_t twi_asyncWriteTo(uint8_t, uint8_t*, uint8_t, uint8_t);
  uint8_t twi_asyncReadFrom(uint8_t, uint8_t*, uint8_t, uint8_t);
  uint8_t twi_lastAsyncOpStatus();

#endif
