#ifndef STRING_PARSER_INCLUDED
#define STRING_PARSER_INCLUDED

#include <Arduino.h>

class StringParser {
  byte *buffer;
  byte parsePtr;
  byte packetSize;
  boolean success;

  public:
    // initialize parser over buffer
    void init(byte *buffer);
    // start parsing from the beginning of buffer
    void reset(byte length);
    
    // parsing commands
    void skipWhitespace();
    byte readChar();
    byte readByte();
    unsigned int readInt();
    int readSignedInt();
    unsigned long readLong();
    
    // final check
    bool stringParsed();
    
    // report error and buffer status
    void parseError(int code);

  private:
    boolean checkValidity();
};

extern StringParser Parser;

#endif