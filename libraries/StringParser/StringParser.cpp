#include <StringParser.h>

boolean isSpace(byte nextChar) {
  return nextChar==' ' || nextChar=='\n' || nextChar=='\t';
}

void StringParser::init(byte *aBuffer) {
  buffer = aBuffer;
}

void StringParser::reset(byte readLength) {
  success = true;
  packetSize = readLength;
  parsePtr = 0;
}
    
void StringParser::skipWhitespace() {
  if (!checkValidity()) return;
  while(parsePtr<packetSize) {
    byte nextChar = buffer[parsePtr];
    if (!isSpace(nextChar)) return;
    parsePtr++;
  }
  success = false;
}

byte StringParser::readChar() {
  if (!checkValidity()) return 0;
  if (parsePtr==packetSize) {
    success = false;
    return 0;
  }
  return buffer[parsePtr++];
}

byte StringParser::readByte() {
  if (!checkValidity()) return 0;
  byte result = 0;
  for(;parsePtr<packetSize && !isSpace(buffer[parsePtr]);parsePtr++) {
    result = result * 10 + (buffer[parsePtr] - '0');
  }
  return result;
}

unsigned int StringParser::readInt() {
  if (!checkValidity()) return 0;
  unsigned int result = 0;
  for(;parsePtr<packetSize && !isSpace(buffer[parsePtr]);parsePtr++) {
    result = result * 10 + (buffer[parsePtr] - '0');
  }
  return result;
}

int StringParser::readSignedInt() {
  if (!checkValidity()) return 0;
  int result = 0;
  int sign = 1;
  if (buffer[parsePtr]=='-') {
    sign = -1;
    parsePtr++;
  }
  for(;parsePtr<packetSize && !isSpace(buffer[parsePtr]);parsePtr++) {
    result = result * 10 + (buffer[parsePtr] - '0');
  }
  return result * sign;
}

unsigned long StringParser::readLong() {
  if (!checkValidity()) return 0;
  unsigned long result = 0;
  for(;parsePtr<packetSize && !isSpace(buffer[parsePtr]);parsePtr++) {
    result = result * 10 + (buffer[parsePtr] - '0');
  }
  return result;
}

bool StringParser::stringParsed() {
  return success;
}

void StringParser::parseError(int code) {
  Serial.print("Parse error:");
  Serial.println(code);
  Serial.print("parsePtr = ");
  Serial.print(parsePtr);
  Serial.print("\npacketSize = ");
  Serial.println(packetSize);
  Serial.print("First char = ");
  Serial.println((char)buffer[0]);
}

boolean StringParser::checkValidity() {
  if (!success) return false;
  if (parsePtr==packetSize) {
    success = false;
    return false;
  }
  return true;
}

StringParser Parser = StringParser();
