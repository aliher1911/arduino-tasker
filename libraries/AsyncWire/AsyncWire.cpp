#include <AsyncWire.h>

// init wire (in master mode)
void AsyncWire::init() {
}

// send byte and terminate communication
void AsyncWire::send(uint8_t address, uint8_t data) {
}

// send array and terminate communication
void AsyncWire::send(uint8_t address, const uint8_t *data, size_t length) {
}

// request size bytes from address
void AsyncWire::read(uint8_t address, uint8_t size, uint8_t *buffer) {
}

// read a single byte
void AsyncWire::read(uint8_t address) {
}

// status check. if true last operation is finished
// and transmission status and/or data could be read
boolean AsyncWire::isReady() {
}

// 0 - success of last op, !=0 various errors
unit8_t AsyncWire::getStatus() {
}

// last byte retrieved by successful read. could be used without buffer
// for small transmissions like single register reading
uint8_t AsyncWire::getLastByte() {
}

AsyncWire Wire = AsyncWire();
