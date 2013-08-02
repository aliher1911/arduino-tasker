#include <Triggers.h>

void ResourceTrigger::init(byte tag) {
  resourceTag  =  tag;
  resourceMask = ~tag;
  status = false;
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

void SerialTrigger::init(byte tag) {
  resourceTag = tag;
}

byte SerialTrigger::setTrigger(byte event) {
  if (Serial.available()) {
    event |= resourceTag;
  }
  return event;
}

byte SerialTrigger::updateTrigger(byte event) {
  return event;
}

void CaptureResource::init(void (*aCallback)(Task *task, byte handle)) {
  callback = aCallback;
}

void CaptureResource::start(byte aHandle) {
  handle = aHandle;
}

void CaptureResource::doTask(Task *task, byte trigger, unsigned long time) {
  task->trigger = 0;
  callback(task, handle);
  if (!task->trigger) task->clear(); // callback didn't reuse we may remove task  
}
