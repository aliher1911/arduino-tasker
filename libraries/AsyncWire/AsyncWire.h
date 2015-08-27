#ifndef ASYNC_WIRE_INCLUDED
#define ASYNC_WIRE_INCLUDED

#include <Arduino.h>

// enable this to compile debugging methods in class to read misc info from twi lib
// #define ASYNC_DEBUG_METHODS

#ifndef BUF_SIZE
#define BUF_SIZE (8)
#endif

#define WIRE_ASYNC_SEND_BUFFER_OVERSHOT (6)
#define WIRE_ASYNC_RECV_BUFFER_OVERSHOT (7)
#define WIRE_ASYNC_NO_DATA              (8)

class AsyncWire {
  private:
    // temporary buffer to send data if no external is provided
    static uint8_t sendBuf[BUF_SIZE];
    
    // current incoming buffer reference
    uint8_t *rxBuffer;
    uint8_t rxBufferIndex;  // current read index for received data (not used by wire op, but by readByte)
    uint8_t rxBufferLength; // rx buffer length. set to internal or external buff size
    uint8_t rxRequested;    // number of bytes requested (read limit)
    
    // temporary buffer to receive data if no external is provided
    static uint8_t recvBuf[BUF_SIZE];

    // current outgoing buffer reference
    uint8_t *txBuffer;
    uint8_t txBufferLength;
    uint8_t txBufferIndex;
    
    // async op address
    uint8_t address;

  public:
    // init wire (in master mode)
    void init();

    // start exchange with address and initialise pointers to internal buffers 
    void begin(uint8_t address);
    // start send using external buffers of length
    // 0 - success 
    int8_t addSend(uint8_t *data, size_t length);
    // add byte to buffer (internal, for external add byte would always cause overflow)
    // 0 - success, 1 - buffer exhausted
    int8_t addSend(uint8_t data);
    // add two bytes to buffer (internal, for external add byte would always cause overflow)
    // 0 - success, 1 - buffer exhausted
    int8_t addSend(uint8_t data1, uint8_t data2);
    // add receive request using external buffer
    // 0 - success
    int8_t addReceive(uint8_t *data, size_t length);
    // add receive request using internal buffer
    // 0 - success, 1 - buffer overshot
    int8_t addReceive(size_t length);
    // perform async operation
    // 0 - success,
    // !0 - wire is busy
    int8_t doAsync();
    // perform sync operation
    // sync schedules async and them waits for operation to succeed in a while loop
    // checking status
    // 0 - success, !0 - bus error
    int8_t doSync();
    
    // send byte and terminate communication
    int8_t send(uint8_t address, uint8_t data);
    // send two bytes and terminate communication
    int8_t send(uint8_t address, uint8_t data1, uint8_t data2);
    // send array and terminate communication
    int8_t send(uint8_t address, uint8_t *data, size_t length);
    // request size bytes from address
    int8_t read(uint8_t address, uint8_t size, uint8_t *buffer);
    // read a single byte
    int8_t read(uint8_t address);
    
    // status check. if true last operation is finished
    // and transmission status and/or data could be read
    boolean isReady();
    // 0 - success of last op, !=0 various errors
    uint8_t getStatus();

#ifdef ASYNC_DEBUG_METHODS
    // get status of underlying library
    uint8_t twiStatus();
    // get status of underlying library
    uint8_t twiEvent();
    // last op address
    uint8_t twiAddress();
#endif
    
    // next byte in receive buffer. could be used without internal or external buffer
    // reading advances internal read pointer for array, but not beyond buffer boundary
    // unsigned byte or -1 if no more bytes to read
    int getNextByte();
    // true if bytes could be read from receive buffer
    // note that this value could be used only after successful read operation
    // in other cases it can return true if receive array pointer is less than buffer size
    // but regardless of what is actually in buffer
    boolean available();
};

extern AsyncWire Wire;

#endif
