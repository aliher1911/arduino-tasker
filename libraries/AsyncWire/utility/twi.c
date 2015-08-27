/*
  twi.c - TWI/I2C library for Wiring & Arduino
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

  Modified 2012 by Todd Krein (todd@krein.org) to implement repeated starts
  Modified 2013 by Oleg Afanasiev (oafanasiev@gmail.com) to implement async operations
*/

#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <compat/twi.h>
#include "Arduino.h" // for digitalWrite

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include "pins_arduino.h"
#include "twi.h"

static volatile uint8_t twi_state;              // wire status
static volatile uint8_t twi_slarw;              // request device address
static volatile uint8_t twi_sendStop;			// should the transaction end with a stop
static volatile uint8_t twi_inRepStart;			// in the middle of a repeated start

static uint8_t *twi_masterSendBuffer;
static uint8_t *twi_masterRecvBuffer;
static volatile uint8_t twi_masterBufferIndex;
static volatile uint8_t twi_masterSendBufferLength;
static volatile uint8_t twi_masterRecvBufferLength;

static volatile uint8_t twi_error;              // error returned by last op

static volatile uint8_t twi_asyncStatus;        // async operation status

static volatile uint8_t twi_event;              // last interrupt event received

/* 
 * Function twi_init
 * Desc     readys twi pins and sets twi bitrate
 * Input    none
 * Output   none
 */
void twi_init(void)
{
  // initialize state
  twi_state = TWI_READY;
  twi_sendStop = true;		// default value
  twi_inRepStart = false;
  
  // activate internal pullups for twi.
  digitalWrite(SDA, 1);
  digitalWrite(SCL, 1);

  // initialize twi prescaler and bit rate
  cbi(TWSR, TWPS0);
  cbi(TWSR, TWPS1);
  TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;

  /* twi bit rate formula from atmega128 manual pg 204
  SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
  note: TWBR should be 10 or higher for master mode
  It is 72 for a 16mhz Wiring board with 100kHz TWI */

  // enable twi module, acks, and twi interrupt
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
}

/* 
 * Function twi_reply
 * Desc     sends byte or readys receive line
 * Input    ack: byte indicating to ack or to nack
 * Output   none
 */
void twi_reply(uint8_t ack)
{
  // transmit master read ready signal, with or without ack
  if(ack){
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
  }else{
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
  }
}

/* 
 * Function twi_stop
 * Desc     relinquishes bus master status
 * Input    none
 * Output   none
 */
void twi_stop(void)
{
  // send stop condition
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTO);

  if (twi_asyncStatus) {
    // mark async as stop pending
    twi_asyncStatus = TWI_AWAIT_STOP;
  } else {
    // wait for stop condition to be executed on bus
    // TWINT is not set after a stop condition!
    while(TWCR & _BV(TWSTO)){
      continue;
    }

    // update twi state
    twi_state = TWI_READY;
  }
}

/* 
 * Function twi_releaseBus
 * Desc     releases bus control
 * Input    none
 * Output   none
 */
void twi_releaseBus(void)
{
  // release bus
  TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT);

  // update twi state and async state
  twi_state = TWI_READY;
  twi_asyncStatus = TWI_COMPLETE;
}

// schedule data for sending
// result 0 - send/receive scheduled
// result 5 - not ready to send
uint8_t twi_asyncWriteRead(uint8_t address, 
                           uint8_t* sendData, uint8_t sendLength,
                           uint8_t* recvData, uint8_t recvLength,
                           uint8_t sendStop)
{
  if (TWI_COMPLETE != twi_asyncStatus) {
    return TWI_ASYNC_BUSY;
  }

  twi_state = TWI_MTX;
  twi_sendStop = sendStop;
  // reset error state (0xFF.. no error occured)
  twi_error = 0xFF;

  // initialise buffers
  twi_masterSendBuffer = sendData;
  twi_masterBufferIndex = 0;
  twi_masterSendBufferLength = sendLength;
  twi_masterRecvBuffer = recvData;
  twi_masterRecvBufferLength = recvLength - 1; // see comments in twi_asyncReadFrom
  
  // build sla+w, slave device address + w bit
  twi_slarw = TW_WRITE;
  twi_slarw |= address << 1;
  
  // set async processing flags
  twi_asyncStatus = TWI_PENDING;
  
  // if we're in a repeated start, then we've already sent the START
  // in the ISR. Don't do it again.
  //
  if (true == twi_inRepStart) {
    // if we're in the repeated start state, then we've already sent the start,
    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
    // We need to remove ourselves from the repeated start state before we enable interrupts,
    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
    // up. Also, don't enable the START interrupt. There may be one pending from the 
    // repeated start that we sent outselves, and that would really confuse things.
    twi_inRepStart = false;			// remember, we're dealing with an ASYNC ISR
    TWDR = twi_slarw;				
    TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);	// enable INTs, but not START
  }
  else
    // send start condition
    TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);	// enable INTs

  // now we are sending data asynchronously. let interrupt handler do the work
  return TWI_ASYNC_SCHEDULED;
}

// schedule data for sending
// result 0 - send scheduled
// result 2 - not ready to send
uint8_t twi_asyncWriteTo(uint8_t address, uint8_t* data, uint8_t length, uint8_t sendStop)
{
  if (TWI_COMPLETE != twi_asyncStatus) {
    return TWI_ASYNC_BUSY;
  }

  twi_state = TWI_MTX;
  twi_sendStop = sendStop;
  // reset error state (0xFF.. no error occured)
  twi_error = 0xFF;

  // initialize buffer iteration vars
  twi_masterSendBuffer = data;
  twi_masterBufferIndex = 0;
  twi_masterSendBufferLength = length;
  twi_masterRecvBuffer = 0; // this is an indication that we don't want to request after send
  
  // build sla+w, slave device address + w bit
  twi_slarw = TW_WRITE;
  twi_slarw |= address << 1;
  
  // set async processing flags
  twi_asyncStatus = TWI_PENDING;
  
  // if we're in a repeated start, then we've already sent the START
  // in the ISR. Don't do it again.
  //
  if (true == twi_inRepStart) {
    // if we're in the repeated start state, then we've already sent the start,
    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
    // We need to remove ourselves from the repeated start state before we enable interrupts,
    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
    // up. Also, don't enable the START interrupt. There may be one pending from the 
    // repeated start that we sent outselves, and that would really confuse things.
    twi_inRepStart = false;			// remember, we're dealing with an ASYNC ISR
    TWDR = twi_slarw;				
    TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);	// enable INTs, but not START
  }
  else
    // send start condition
    TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);	// enable INTs

  // now we are sending data asynchronously. let interrupt handler do the work
  return TWI_ASYNC_SCHEDULED;
}

// request data from device
// result 0 - receive scheduled
// result 2 - not ready to receive
uint8_t twi_asyncReadFrom(uint8_t address, uint8_t* data, uint8_t length, uint8_t sendStop)
{
  // wait until twi is ready, become master receiver
  if (TWI_COMPLETE != twi_asyncStatus) {
    return TWI_ASYNC_BUSY;
  }
  
  twi_state = TWI_MRX;
  twi_sendStop = sendStop;
  // reset error state (0xFF.. no error occured)
  twi_error = 0xFF;

  // initialise buffer iteration vars
  twi_masterRecvBuffer = data;
  twi_masterBufferIndex = 0;
  twi_masterRecvBufferLength = length-1;  // This is not intuitive, read on...
  // On receive, the previously configured ACK/NACK setting is transmitted in
  // response to the received byte before the interrupt is signalled. 
  // Therefor we must actually set NACK when the _next_ to last byte is
  // received, causing that NACK to be sent in response to receiving the last
  // expected byte of data.
  twi_masterSendBuffer = 0; // this is an indication that we don't want to request after send

  // build sla+w, slave device address + w bit
  twi_slarw = TW_READ;
  twi_slarw |= address << 1;

  // set async processing flags
  twi_asyncStatus = TWI_PENDING;

  if (true == twi_inRepStart) {
    // if we're in the repeated start state, then we've already sent the start,
    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
    // We need to remove ourselves from the repeated start state before we enable interrupts,
    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
    // up. Also, don't enable the START interrupt. There may be one pending from the 
    // repeated start that we sent outselves, and that would really confuse things.
    twi_inRepStart = false;			// remember, we're dealing with an ASYNC ISR
    TWDR = twi_slarw;
    TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);	// enable INTs, but not START
  }
  else
    // send start condition
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);

  // scheduled. status could be checked for completion
  return TWI_ASYNC_SCHEDULED;
}

uint8_t twi_lastAsyncOpStatus(void)
{
  switch(twi_asyncStatus) {
    case TWI_PENDING:
      // operation still in progress
      return TWI_ASYNC_INPROGRESS;
    case TWI_AWAIT_STOP:
      // if stop is not sent return pending
      if (TWCR & _BV(TWSTO)) return TWI_ASYNC_INPROGRESS;
      twi_asyncStatus = TWI_COMPLETE;
      // set sync status to ready
      twi_state = TWI_READY;
    case TWI_COMPLETE:
      if (twi_error == 0xFF)
        return TWI_ASYNC_SUCCESS;
      else if (twi_error == TW_MT_SLA_NACK)
        return TWI_ASYNC_ADDR_NACK;
      else if (twi_error == TW_MT_DATA_NACK)
        return TWI_ASYNC_DATA_NACK;
      else
        return TWI_ASYNC_BUS_ERROR;
  }
}

uint8_t twi_status(void) {
  return twi_state;
}

uint8_t twi_lastEvent(void) {
  return twi_event;
}

uint8_t twi_lastAddr(void) {
  return twi_slarw;
}

SIGNAL(TWI_vect)
{
  twi_event = TW_STATUS;
  
  switch(TW_STATUS){
    // All Master
    case TW_START:     // sent start condition
    case TW_REP_START: // sent repeated start condition
      // copy device address and r/w bit to output register and ack
      TWDR = twi_slarw;
      twi_reply(1);
      break;

    // Master Transmitter
    case TW_MT_SLA_ACK:  // slave receiver acked address
    case TW_MT_DATA_ACK: // slave receiver acked data
      // if there is data to send, send it, otherwise stop 
      if (twi_masterBufferIndex < twi_masterSendBufferLength) {
        // copy data to output register and ack
        TWDR = twi_masterSendBuffer[twi_masterBufferIndex++];
        twi_reply(1);
      } else {
        if (twi_masterRecvBuffer==0) {
          if (twi_sendStop)
            twi_stop();
          else {
            twi_inRepStart = true;	// we're gonna send the START
            // don't enable the interrupt. We'll generate the start, but we 
            // avoid handling the interrupt until we're in the next transaction,
            // at the point where we would normally issue the start.
            TWCR = _BV(TWINT) | _BV(TWSTA)| _BV(TWEN) ;
            twi_state = TWI_READY;
            twi_asyncStatus = TWI_COMPLETE;
          }
        } else {
          // start receiver
          twi_masterBufferIndex = 0;
          twi_state = TWI_MRX;
          // prepare address (old address + read bit)
          twi_slarw &= 0xFE;
          twi_slarw |= TW_READ;
          // send repeated start to initiate read
          TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE)  | _BV(TWSTA);
        }
      }
      break;
    case TW_MT_SLA_NACK:  // address sent, nack received
      twi_error = TW_MT_SLA_NACK;
      twi_stop();
      break;
    case TW_MT_DATA_NACK: // data sent, nack received
      twi_error = TW_MT_DATA_NACK;
      twi_stop();
      break;
    case TW_MT_ARB_LOST: // lost bus arbitration
      twi_error = TW_MT_ARB_LOST;
      twi_releaseBus();
      break;

    // Master Receiver
    case TW_MR_DATA_ACK: // data received, ack sent
                         // put byte into buffer
      twi_masterRecvBuffer[twi_masterBufferIndex++] = TWDR;
    case TW_MR_SLA_ACK:  // address sent, ack received
      // ack if more bytes are expected, otherwise nack
      if(twi_masterBufferIndex < twi_masterRecvBufferLength) {
        twi_reply(1);
      } else {
        twi_reply(0);
      }
      break;
    case TW_MR_DATA_NACK: // data received, nack sent
                          // put final byte into buffer
      twi_masterRecvBuffer[twi_masterBufferIndex++] = TWDR;
      if (twi_sendStop)
          twi_stop();
      else {
	      twi_inRepStart = true;	// we're gonna send the START
        // don't enable the interrupt. We'll generate the start, but we 
        // avoid handling the interrupt until we're in the next transaction,
        // at the point where we would normally issue the start.
        TWCR = _BV(TWINT) | _BV(TWSTA)| _BV(TWEN) ;
        twi_state = TWI_READY;
        twi_asyncStatus = TWI_COMPLETE;
      }    
      break;
    case TW_MR_SLA_NACK: // address sent, nack received
      twi_stop();
      break;
    // TW_MR_ARB_LOST handled by TW_MT_ARB_LOST case

    // All
    case TW_NO_INFO:   // no state information
      break;
    case TW_BUS_ERROR: // bus error, illegal stop/start
      twi_error = TW_BUS_ERROR;
      twi_stop();
      break;
  }
}

