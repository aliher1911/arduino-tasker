#ifndef SERIAL_READER_TASK_INCLUDED
#define SERIAL_READER_TASK_INCLUDED

#include <TaskManager.h>
#include <Triggers.h>

class SerialTrigger : public Trigger {
  byte resourceTag;
 
  public:
    // init trigger and set serial tag
    void init(byte tag);
    // get trigger associated with this resource
    byte trigger();
    // set trigger at the beginning of loop
    virtual byte setTrigger(byte event);
    // update trigger after each task
    virtual byte updateTrigger(byte event);
};

class SerialReaderTask : public TaskHandler {
  // buffer for command reading
  byte *serialBuffer;
  // bytes already read
  byte packetSize;
  // parser function pointer
  void (*function)(int size);

  public:
    void init(byte *buffer, void (*function)(int size));
    void start(byte id);
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

// create one for each task needing response
class SerialResponseTask : public TaskHandler {
  int value;
  
  public:
    void start(byte id, int value);
    void start(Task *actionTask, int value);
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

class SerialReleaseTask : public TaskHandler {

  public:
    void start(Task *actionTask, unsigned short delay);
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

class PacketSendTask : public TaskHandler {
  byte *buffer;
  unsigned short ptr;           // current send pointer
  unsigned short packetSize;    // bytes send in one packet
  unsigned short bufferSize;    // number of bytes to send
  unsigned long  timeStep;      // how often to send bytes

  public:
    void start(byte id, byte *buffer, unsigned short packetSize, unsigned short bufferLength, unsigned long timePeriod);
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

extern SerialTrigger SerialInTrigger;
extern SerialReaderTask SerialTask;
extern ResourceTrigger SerialOutSemaphore;
extern PacketSendTask PacketTask;

#endif
