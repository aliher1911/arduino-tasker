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
    // update trigger status
    void aquire();
    void release();
    // is trigger on
    virtual boolean isOn();
    // get trigger associated with this resource
    virtual byte trigger();
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
    void init(ResourceTrigger *trigger, void (*callback)(Task *task, byte handle));
    void start(byte id, byte handle);
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

#endif
