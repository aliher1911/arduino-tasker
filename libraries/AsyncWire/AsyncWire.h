#ifndef ASYNC_WIRE_INCLUDED
#define ASYNC_WIRE_INCLUDED

#include <Arduino.h>

class AsyncWire {
  private:
    uint8_t buf[2];
    uint8_t *incoming;
    uint8_t *outgoing;

  public:
    // init wire (in master mode)
    void init();
    // prepare transmission (just record address)
    void prepareSend(uint8_t address);
    // send byte and terminate communication
    void send(uint8_t data);
    // send array and terminate communication
    void send(const uint8_t *data, size_t length);
    // request size bytes from address
    void read(uint8_t address, uint8_t size, uint8_t *buffer);
    // read a single byte
    void read(uint8_t address);
    // status check. if true last operation is finished
    // and transmission status and/or data could be read
    boolean isReady();
    // 0 - success of last op, !=0 various errors
    unit8_t getStatus();
    // last byte retrieved by successful read. could be used without buffer
    // for small transmissions like single register reading
    uint8_t getLastByte();
};

extern AsyncWire Wire;

#endif

