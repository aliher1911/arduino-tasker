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
  
  Modified 2013 by Oleg Afanasiev (oafanasiev@gmail.com) to implement async operations
*/

#ifndef twi_h
#define twi_h

  #include <inttypes.h>

  //#define ATMEGA8

  #ifndef TWI_FREQ
  #define TWI_FREQ 100000L
  #endif

  // scheduling errors
  #define TWI_ASYNC_SCHEDULED  (0)
  #define TWI_ASYNC_BUSY       (5)

  // async statuses
  #define TWI_ASYNC_INPROGRESS (0)
  #define TWI_ASYNC_SUCCESS    (1)
  #define TWI_ASYNC_ADDR_NACK  (2)
  #define TWI_ASYNC_DATA_NACK  (3)
  #define TWI_ASYNC_BUS_ERROR  (4)

  // this is not really used anymore
  #define TWI_READY 0
  #define TWI_MRX   1
  #define TWI_MTX   2
  
  // async operation status
  #define TWI_COMPLETE    0
  #define TWI_PENDING     1
  #define TWI_AWAIT_STOP  2

  // initialize wire interface    
  void twi_init(void);

  // internally used functions
  // tell wire to send or acknowledge data in data register
  void twi_reply(uint8_t);
  // send stop after wire op
  void twi_stop(void);
  // release bus after wire op
  void twi_releaseBus(void);
  
  // schedule send then receive information
  uint8_t twi_asyncWriteRead(uint8_t, uint8_t*, uint8_t, uint8_t*, uint8_t, uint8_t);
  // schedule send information
  uint8_t twi_asyncWriteTo(uint8_t, uint8_t*, uint8_t, uint8_t);
  // schedule receive information
  uint8_t twi_asyncReadFrom(uint8_t, uint8_t*, uint8_t, uint8_t);

  // last operation status
  // see async statuses in this file for possible error values
  uint8_t twi_lastAsyncOpStatus(void);
  
  // current transmission status, useful for debugging
  uint8_t twi_status(void);
  // last interrupt event received
  uint8_t twi_lastEvent(void);
  // current address (+encoded op)
  uint8_t twi_lastAddr(void);

#endif

