arduino-tasker
==============

AsyncWire
=========

Replacement for Wire to allow async communication.

Init I2C in master mode, call once in setup to init library and underlying transport.
wire.init();

Sending data
------------
wire.begin(address);

If using external buffer use:
wire.addSend(data, lenght);

If using preallocated internal buffer of 8 bytes:
wire.addSend(byte);
could be called multiple times to fill packet.

When packet is ready:
wire.doAsync();
intiate communication (will only succeed if no operation is in progress)

after that do
while(!wire.isReady()) {
  // wait for operation to complete
}
to check result of operation after wire is ready
wire.getStatus();
will return 0 if operation was successfull

Receiving data
--------------
If using external buffer
wire.addReceive(buffer, length);

If using preallocated internal buffer of 8 bytes:
wire.addReceive(length);

Once parameters are set call
wire.doAsync();
to initiate communication. Follow send pattern after that.

To read received data byte by byte
while(wire.available()) {
  wire.getNextByte();
}

Send and Receive data
---------------------
Setup send and receive and then initiate communication.

Synchronous mode
----------------
If sync operation is needed, then method
wire.doSync();
will initiate communication and wait till it is complete (send, receive or both)

Shortcuts
---------
Methods send and read with multiple flavours will setup communication according to arguments
and perform synchronous op.
Handy when short exchanges are needed with fixed 1-2 byte packets.

