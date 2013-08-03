#include <SerialTasks.h>

void SerialReaderTask::init(byte *aBuffer, void (*aFunction)(int size)) {
  packetSize = 0;
  function = aFunction;
  serialBuffer = aBuffer;
}

void SerialReaderTask::start(byte id) {
  TM.addTask(2, SerialInTrigger.trigger(), this);
}

void SerialReaderTask::doTask(Task *task, byte trigger, unsigned long time) {
  int incoming = Serial.read();
  if (incoming=='\n' || incoming=='\r' || packetSize==80) {
    // end packet
    function(packetSize);
    packetSize = 0;
  } else {
    serialBuffer[packetSize++] = incoming;
  }
}

SerialReleaseTask ReleaseTask = SerialReleaseTask();

void SerialResponseTask::start(byte id, int aValue) {
  value = aValue;
}

void SerialResponseTask::start(Task *actionTask, int aValue) {
  value = aValue;
}

void SerialResponseTask::doTask(Task *task, byte trigger, unsigned long time) {
  SerialOutSemaphore.aquire();
  Serial.print("k ");
  Serial.print(task->id);
  Serial.print(' ');
  Serial.println(value);
  ReleaseTask.start(task, 100); // 1 byte/ms on 9600, 64 byte buffer max
}

void SerialReleaseTask::start(Task *task, unsigned short delay) {
  task->trigger = TIME_TRIGGER;
  task->time = millis() + delay; 
  task->handler = this;
}

void SerialReleaseTask::doTask(Task *task, byte trigger, unsigned long time) {
  task->clear();
  SerialOutSemaphore.release();
}

void PacketSendTask::start(byte id, byte *aBuffer, unsigned short aPacketSize, 
                           unsigned short aBufferLength, unsigned long aTimePeriod) {
  ptr = 0;
  buffer = aBuffer;
  packetSize = aPacketSize;
  bufferSize = aBufferLength;
  timeStep = aTimePeriod;
  TM.addTask(id, SerialOutSemaphore.trigger(), this);
}

void PacketSendTask::doTask(Task *task, byte trigger, unsigned long time) {
  if (ptr==0) {
    // begin packet
    SerialOutSemaphore.aquire();
    task->trigger = TIME_TRIGGER;
    Serial.print("t ");
    Serial.print(task->id);
  }
  int endPtr = ptr + packetSize;
  if (endPtr>bufferSize) endPtr = bufferSize;
  for(;ptr<endPtr;ptr++) {
    Serial.print(' ');
    Serial.print(buffer[ptr]);
  }
  if (ptr==bufferSize) {
    Serial.println();
    // last packet, we need to release serial after timeout
    ReleaseTask.start(task, timeStep);
  } else {
    task->time += timeStep;
  }
}

void SerialTrigger::init(byte tag) {
  resourceTag = tag;
  TM.registerTrigger(this);
}

byte SerialTrigger::trigger() {
  return resourceTag;
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

SerialTrigger SerialInTrigger = SerialTrigger();
SerialReaderTask SerialTask = SerialReaderTask();
ResourceTrigger SerialOutSemaphore = ResourceTrigger();
PacketSendTask PacketTask = PacketSendTask();
