#include <AsyncWire.h>

extern "C" {
  #include <twi.h>
}

uint8_t AsyncWire::sendBuf[BUF_SIZE];
uint8_t AsyncWire::recvBuf[BUF_SIZE];

// init wire (in master mode)
void AsyncWire::init() {
  // reset buffers to prevent corruption
  rxBufferIndex = 0;
  rxBufferLength = 0;
  txBufferIndex = 0;
  txBufferLength = 0;
  // init library
  twi_init();
}

// start exchange with address and initialise pointers to internal buffers 
void AsyncWire::begin(uint8_t anAddress) {
  address = anAddress;
  rxBuffer = recvBuf;
  rxBufferLength = BUF_SIZE;
  rxBufferIndex = 0;
  txBuffer = sendBuf;
  txBufferLength = BUF_SIZE;
  txBufferIndex = 0;
}

// start send using external buffers of length
// 0 - success
int8_t AsyncWire::addSend(uint8_t *data, size_t length) {
  txBuffer = data;
  txBufferLength = length;
  txBufferIndex = length;
  return 0;
}

// add byte to buffer (internal or external at current pointer)
// 0 - success, 6 - buffer exhausted
int8_t AsyncWire::addSend(uint8_t data) {
  if (txBufferIndex>=txBufferLength) return WIRE_ASYNC_SEND_BUFFER_OVERSHOT;
  txBuffer[txBufferIndex++] = data;
  return 0;
}

// add two bytes to buffer (internal or external at current pointer)
// useful when you need to write to an addressed register
// 0 - success, 6 - buffer exhausted
int8_t AsyncWire::addSend(uint8_t data1, uint8_t data2) {
  if (txBufferIndex>=(txBufferLength-1)) return WIRE_ASYNC_SEND_BUFFER_OVERSHOT;
  txBuffer[txBufferIndex++] = data1;
  txBuffer[txBufferIndex++] = data2;
  return 0;
}

// add receive request using external buffer
// 0 - success
int8_t AsyncWire::addReceive(uint8_t *data, size_t length) {
  rxBuffer = data;
  rxRequested = length;
  return 0;
}

// add receive request using internal buffer
// 0 - success, 1 - buffer overshot
int8_t AsyncWire::addReceive(size_t length) {
  if (length>rxBufferLength) return WIRE_ASYNC_RECV_BUFFER_OVERSHOT;
  rxRequested = length;
  return 0;
}

// perform async operation
// 0 - success,
// 1 - no data provided
int8_t AsyncWire::doAsync() {
  if (rxRequested==0 && txBufferIndex==0) {
    return WIRE_ASYNC_NO_DATA;
  }
  if (rxRequested==0) {
    // send only
    return twi_asyncWriteTo(address, txBuffer, txBufferIndex, 1);
  } else if (txBufferIndex==0) {
    // receive only
    return twi_asyncReadFrom(address, rxBuffer, rxRequested, 1);
  } else {
    // send then receive
    return twi_asyncWriteRead(address, txBuffer, txBufferIndex, rxBuffer, rxRequested, 1);
  }
}

// 0 - success
// !0 - communication error
int8_t AsyncWire::doSync() {
  uint8_t result = doAsync();
  if (result) return result;
  while(!twi_lastAsyncOpStatus()) {} // spin while not complete
  uint8_t status = getStatus();
  return status==TWI_ASYNC_SUCCESS?0:status;
}

// send byte and terminate communication
// device address, byte to send
int8_t AsyncWire::send(uint8_t address, uint8_t data) {
  // use temp buffer to send data
  sendBuf[0] = data;
  return send(address, sendBuf, 1);
}

// send two bytes and terminate communication
// useful when writing a value into device register
// device address, 2 bytes to send
int8_t AsyncWire::send(uint8_t address, uint8_t data1, uint8_t data2) {
  // use temp buffer to send data
  sendBuf[0] = data1;
  sendBuf[1] = data2;
  return send(address, sendBuf, 2);
}

// send array and terminate communication
int8_t AsyncWire::send(uint8_t address, uint8_t *data, size_t length) {
  return twi_asyncWriteTo(address, data, length, 1);
}

// request size bytes from address
int8_t AsyncWire::read(uint8_t address, uint8_t size, uint8_t *buffer) {
  return twi_asyncReadFrom(address, buffer, size, 1);
}

// read a single byte
int8_t AsyncWire::read(uint8_t address) {
  return twi_asyncReadFrom(address, recvBuf, 1, 1);
}

// status check. if true last operation is finished
// and transmission status and/or data could be read
boolean AsyncWire::isReady() {
  return twi_lastAsyncOpStatus()!=0;
}

// 0 - success of last op, !=0 various errors
uint8_t AsyncWire::getStatus() {
  return twi_lastAsyncOpStatus();
}

#ifdef ASYNC_DEBUG_METHODS

uint8_t AsyncWire::twiStatus() {
  return twi_status();
}

uint8_t AsyncWire::twiEvent() {
  return twi_lastEvent();
}

uint8_t AsyncWire::twiAddress() {
  return twi_lastAddr();
}

#endif

// last byte retrieved by successful read. could be used without buffer
// for small transmissions like single register reading
// unsigned byte or -1 if no more bytes to read
int AsyncWire::getNextByte() {
  if (rxBufferIndex>=rxBufferLength) return -1;    // signed error response
  return ((int)recvBuf[rxBufferIndex++] & 0xff); // unsigned byte
}

boolean AsyncWire::available() {
  return rxBufferIndex<rxBufferLength;
}

AsyncWire Wire = AsyncWire();
