#ifndef TRIGGERS_INCLUDED
#define TRIGGERS_INCLUDED

#include <Arduino.h>
#include <TaskManager.h>

class ResourceTrigger : public Trigger {
  byte resourceTag;
  byte resourceMask;
  boolean status;

  public:
    // init trigger and set resource tag
    void init(byte tag);
    // get trigger associated with this resource
    byte trigger();
    // update trigger status
    void aquire();
    void release();
    // set trigger at the beginning of loop
    virtual byte setTrigger(byte event);
    // update trigger after each task
    virtual byte updateTrigger(byte event);
};

class SerialTrigger : public Trigger {
  byte resourceTag;
 
  public:
    // init trigger and set serial tag
    void init(byte tag);
    // set trigger at the beginning of loop
    virtual byte setTrigger(byte event);
    // update trigger after each task
    virtual byte updateTrigger(byte event);
};

class CaptureResource : public TaskHandler {
  ResourceTrigger *trigger;
  byte handle;
  void (*callback)(Task *task, byte handle);

  public:
//    void init(ResourceTrigger *trigger);
//    void init(ResourceTrigger *trigger, void (*callback)(Task *task, byte handle));
    void init(void (*callback)(Task *task, byte handle));
//    void start(void (*callback)(Task *task, byte handle), byte handle);
    void start(byte handle);
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

#endif
