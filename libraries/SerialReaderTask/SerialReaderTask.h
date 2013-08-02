#ifndef SERIAL_READER_TASK_INCLUDED
#define SERIAL_READER_TASK_INCLUDED

#include <TaskManager.h>
#include <Triggers.h>

class SerialReaderTask : public TaskHandler {
  // buffer for command reading
  byte *serialBuffer;
  // bytes already read
  byte packetSize;
  // parser function pointer
  void (*function)(int size);

  public:
    void init(byte *buffer, void (*function)(int size));
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

// create one for each task needing response
class SerialResponseTask : public TaskHandler {
  int value;
  
  public:
    void setResponseCode(int value);
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

class SerialReleaseTask : public TaskHandler {

  public:
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

void createSerialResponseTask(Task *task, int code);

class PacketSendTask : public TaskHandler {
  byte *buffer;
  unsigned short ptr;           // current send pointer
  unsigned short packetSize;    // bytes send in one packet
  unsigned short bufferSize;    // number of bytes to send
  unsigned long  timeStep;      // how often to send bytes

  public:
    void init(byte *buffer, unsigned short packetSize, unsigned short bufferLength, unsigned long timePeriod);
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

extern SerialReaderTask SerialTask;
extern ResourceTrigger SerialSemaphor;
extern PacketSendTask PacketTask;

#endif
