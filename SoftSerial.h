#ifndef SoftSerial_h
#define SoftSerial_h

#define HardwareSerial_h // don't include arduinos hardware serial
#include "Arduino.h"

#include <inttypes.h>

#include "Stream.h"

class SoftSerial : public Stream
{
  protected:
    // Has any byte been written to the UART since begin()
    bool _written;

  public:
    inline SoftSerial() {};
    void begin();
    void begin(long int) { begin(); }
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    int availableForWrite(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() { return true; }
};

#define Serial MySoftSerial
extern SoftSerial MySoftSerial;

#endif
