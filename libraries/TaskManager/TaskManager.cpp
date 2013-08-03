#include <TaskManager.h>

boolean Task::matches(byte aTrigger, unsigned long aTime) {
  if (!(aTrigger & trigger)) return false;
  long timeframe = aTime - time;
  return   !(trigger & TIME_TRIGGER) 
         || ((trigger & TIME_TRIGGER) && timeframe>=0 && timeframe<LATE_TIME_THRESHOLD);
}

void Task::clear() {
  id = 0;
  trigger = 0;
}

void TaskManager::init() {
  // clean triggers so they arent triggered
  int i;
  for(i=0;i<TASK_QUEUE_SIZE;i++) {
    queue[i].clear();
  }
}

Task* TaskManager::addTaskInternal(byte id, byte trigger, unsigned long firstInvocation, TaskHandler *handler) {
  int i;
  for(i=0;i<TASK_QUEUE_SIZE;i++) {
    if (!queue[i].id) {
      queue[i].id = id;
      queue[i].trigger = trigger;
      queue[i].handler = handler;
      queue[i].time = firstInvocation;
      return queue+i;
    }
  }
  return NULL;
}

Task* TaskManager::addTask(byte id, byte trigger, unsigned long invocationDelay, TaskHandler *handler) {
  return addTaskInternal(id, trigger | TIME_TRIGGER, millis()+invocationDelay, handler);
}

Task* TaskManager::addTask(byte id, byte trigger, TaskHandler *handler) {
  return addTaskInternal(id, trigger, 0, handler);
}

void TaskManager::removeTask(byte id) {
  int i;
  for(i=0;i<TASK_QUEUE_SIZE;i++) {
    if (queue[i].trigger && queue[i].id==id) {
      queue[i].clear();
      return;
    }
  }
}

void TaskManager::writeDebugReportSync() {
  Serial.println("Queue report:");
  Serial.print("Time : ");
  Serial.println(millis());
  int i;
  for(i=0;i<TASK_QUEUE_SIZE;i++) {
    Serial.print(i);
    Serial.print(" : ");
    if (queue[i].trigger) {
      Serial.print(queue[i].id);
      Serial.print(",");
      Serial.print(queue[i].trigger, HEX);
      Serial.print(",");
      Serial.println(queue[i].time);
    } else {
      Serial.println("----");
    }
  }
  Serial.print("Event ");
  Serial.println(getEvents(), HEX);
}

void TaskManager::registerTrigger(Trigger *trigger) {
  int i;
  for(i=0;i<MAX_TRIGGERS;i++) {
    if (!triggers[i]) {
      triggers[i] = trigger;
      return;
    }
  }
}

byte TaskManager::getEvents() {
  byte trigger = TIME_TRIGGER;
  int i;
  for(i=0;i<MAX_TRIGGERS && triggers[i];i++) {
    trigger = triggers[i]->setTrigger(trigger);
  }
  return trigger;
}

byte TaskManager::updateEvents(byte trigger) {
  int i;
  for(i=0;i<MAX_TRIGGERS && triggers[i];i++) {
    trigger = triggers[i]->updateTrigger(trigger);
  }
  return trigger;
}
    
void TaskManager::loop() {
  // form trigger bits
  unsigned long time = millis();
  byte trigger = getEvents();
  // find matching commands and dispatch
  int matching = 0;
  int i;
  for(i=0;i<TASK_QUEUE_SIZE;i++) {
    Task *current = queue + i;
    if (current->trigger && current->matches(trigger, time)) {
      current->handler->doTask(current, trigger, time);
      matching++;
      trigger = updateEvents(trigger);
    }
  }
}

TaskManager TM = TaskManager();
