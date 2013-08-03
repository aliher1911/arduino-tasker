#ifndef TASK_MANAGER_INCLUDED
#define TASK_MANAGER_INCLUDED

#include <Arduino.h>

#define LATE_TIME_THRESHOLD (10000)
#define TASK_QUEUE_SIZE     (10)
#define MAX_TRIGGERS        (10)

#define TIME_TRIGGER        (0x01)

class Trigger {
  public:
    // set trigger at the beginning of loop
    virtual byte setTrigger(byte event) = 0;
    // update trigger after each task
    virtual byte updateTrigger(byte event) = 0;
};

class Task;

class TaskHandler {
  public:
    // virtual method to implement in handlers
    virtual void doTask(Task *task, byte trigger, unsigned long time) = 0;
};

class Task {
  public:
    // task id for management. must be unique, but no checks are done
    byte id;
    // event to match
    byte trigger;
    // time when trigger matches
    unsigned long time;
    // replaces command field
    TaskHandler *handler;
    
    // check for the match
    boolean matches(byte trigger, unsigned long time);
    
    // reset task
    void clear();
};

class TaskManager {
  private:
    Task queue[TASK_QUEUE_SIZE];
    // triggers are external to task manager
    Trigger *triggers[MAX_TRIGGERS];

    byte getEvents();
    byte updateEvents(byte status);
    Task* addTaskInternal(byte id, byte trigger, unsigned long firstInvocation, TaskHandler *handler);

  public:
    void init();
    
    // manage tasks
    Task* addTask(byte id, byte trigger, TaskHandler *handler);
    Task* addTask(byte id, byte trigger, unsigned long invocationDelay, TaskHandler *handler);
    void removeTask(byte id);
    void writeDebugReportSync();
    
    // manage triggers
    void registerTrigger(Trigger *trigger);
    
    // main loop
    void loop();
};

extern TaskManager TM;

#endif
