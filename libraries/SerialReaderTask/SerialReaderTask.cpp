#include <SerialReaderTask.h>

void SerialReaderTask::init(byte *aBuffer, void (*aFunction)(int size)) {
  packetSize = 0;
  function = aFunction;
  serialBuffer = aBuffer;
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

void createSerialResponseTask(Task *task, SerialResponseTask *responseTask) {
  task->trigger = SerialSemaphor.trigger();
  task->handler = responseTask;
}

// replace current task with serial semaphore release
void setReleaseSerial(Task *task, unsigned short delay) {
  task->trigger = TIME_TRIGGER;
  task->handler = &ReleaseTask;
  task->time = millis() + delay; 
}

void SerialResponseTask::setResponseCode(int aValue) {
  value = aValue;
}

void SerialResponseTask::doTask(Task *task, byte trigger, unsigned long time) {
  SerialSemaphor.aquire();
  Serial.print("k ");
  Serial.print(task->id);
  Serial.print(' ');
  Serial.println(value);
  setReleaseSerial(task, 100); // 1 byte/ms on 9600, 64 byte buffer max
}

void SerialReleaseTask::doTask(Task *task, byte trigger, unsigned long time) {
  task->clear();
  SerialSemaphor.release();
}

void PacketSendTask::init(byte *aBuffer, unsigned short aPacketSize, unsigned short aBufferLength, unsigned long aTimePeriod) {
  ptr = 0;
  buffer = aBuffer;
  packetSize = aPacketSize;
  bufferSize = aBufferLength;
  timeStep = aTimePeriod;
}

void PacketSendTask::doTask(Task *task, byte trigger, unsigned long time) {
  if (ptr==0) {
    // begin packet
    SerialSemaphor.aquire();
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
    setReleaseSerial(task, timeStep);
  } else {
    task->time += timeStep;
  }
}


SerialReaderTask SerialTask = SerialReaderTask();
ResourceTrigger SerialSemaphor = ResourceTrigger();
PacketSendTask PacketTask = PacketSendTask();
