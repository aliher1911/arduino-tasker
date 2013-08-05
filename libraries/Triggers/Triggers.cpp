#include <Triggers.h>

void ResourceTrigger::init(byte tag) {
  resourceTag  =  tag;
  resourceMask = ~tag;
  status = false;
  TM.registerTrigger(this);
}

boolean ResourceTrigger::isOn() {
  return !status;
}

byte ResourceTrigger::trigger() {
  return resourceTag;
}

void ResourceTrigger::aquire() {
  status = true;
}
    
void ResourceTrigger::release() {
  status = false;
}

byte ResourceTrigger::setTrigger(byte event) {
  if (!status) {
    event |= resourceTag;
  }
  return event;
}

byte ResourceTrigger::updateTrigger(byte event) {
  if (!status) {
    event |= resourceTag;
  } else {
    event &= resourceMask;
  }
  return event;
}

void CaptureResource::init(ResourceTrigger *aTrigger, void (*aCallback)(Task *task, byte handle)) {
  trigger = aTrigger;
  callback = aCallback;
}

void CaptureResource::start(byte id, byte aHandle) {
  handle = aHandle;
  TM.addTask(id, trigger->trigger(), this);
}

void CaptureResource::doTask(Task *task, byte trigger, unsigned long time) {
  task->trigger = 0;
  callback(task, handle);
  if (!task->trigger) task->clear(); // callback didn't reuse we may remove task  
}
