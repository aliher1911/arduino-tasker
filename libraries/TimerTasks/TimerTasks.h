#ifndef TIMER_TASK_INCLUDED
#define TIMER_TASK_INCLUDED

#include <Arduino.h>
#include <TaskManager.h>

class PeriodicTask : public TaskHandler {
  void (*callback)(Task* task, byte handle, unsigned short value, boolean last);
  
  byte handle;
  unsigned short currentVal;
  unsigned short endVal;
  short incrementStep;
  unsigned long timeStep;

  public:
    void init(void (*callback)(Task* task, byte handle, unsigned short value, boolean last));
    void start(byte id, byte handle, unsigned short startVal, unsigned short endVal, 
               short increment, unsigned long timeStep);
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

class TimerTask : public TaskHandler {
  void (*callback)(Task* task, byte handle);
  
  byte handle;

  public:
    void init(void (*callback)(Task* task, byte handle));
    void start(byte id, byte handle, unsigned long invocationDelay);
    void start(Task *task, byte handle, unsigned long invocationDelay);
    virtual void doTask(Task *task, byte trigger, unsigned long time);
};

#endif