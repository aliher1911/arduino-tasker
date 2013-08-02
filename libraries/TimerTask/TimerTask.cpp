#include <TimerTask.h>

void PeriodicTask::init(void (*aCallback)(Task* task, byte handle, unsigned short value, boolean last)) {
  callback = aCallback;
}

void PeriodicTask::start(byte aHandle, unsigned short startVal,
                      unsigned short anEndVal, short increment, unsigned long aTimeStep) {
  handle = aHandle;
  currentVal = startVal;
  endVal = anEndVal;
  incrementStep = increment;
  timeStep = timeStep;
}

void PeriodicTask::doTask(Task *task, byte trigger, unsigned long time) {
  // save current value for notification
  unsigned short value = currentVal;
  // increase position
  currentVal += incrementStep;
  // check if next is still in range
  if (incrementStep>0 && currentVal>endVal || incrementStep<0 && currentVal<endVal) {
    // stop sweep since we overshot
    // reset trigger in case client doesn't clear task correctly,
    // but we still need to preserve id so can't call clear()
    task->trigger = 0;
    // send last response
    callback(task, handle, value, true);
    if (!task->trigger) task->clear(); // callback didn't update we may remove task
  } else {
    callback(task, handle, value, false);
    // increse time for next step
    task->time += timeStep;
  }
}

void TimerTask::init(void (*aCallback)(Task* task, byte handle)) {
  callback = aCallback;
}

void TimerTask::start(byte aHandle) {
  handle = aHandle;
}

void TimerTask::doTask(Task *task, byte trigger, unsigned long time) {
  task->trigger = 0;
  callback(task, handle);
  if (!task->trigger) task->clear(); // callback didn't update we may remove task
}
