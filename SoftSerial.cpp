#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#define HardwareSerial_h // don't include arduinos hardware serial
#include "Arduino.h"

#include "SoftSerial.h"
extern "C" {
	extern void softuart_init();
	extern unsigned char softuart_kbhit( void );
	extern char softuart_getchar( void );
	extern unsigned char softuart_transmit_busy( void );
	extern void softuart_putchar( const char );
	extern void softuart_flush_input_buffer( void );
}

SoftSerial MySoftSerial;

// Public Methods //////////////////////////////////////////////////////////////

void SoftSerial::begin()
{
	softuart_init();
}

void SoftSerial::end()
{
}

int SoftSerial::available(void)
{
  return softuart_kbhit();
}

int SoftSerial::peek(void)
{
 return -1;
}

int SoftSerial::read(void)
{
	if (softuart_kbhit())
		return softuart_getchar();
	return -1;
}

int SoftSerial::availableForWrite(void)
{
	return 1;
}

void SoftSerial::flush()
{
	while (softuart_transmit_busy())
		;
	softuart_flush_input_buffer();
}

size_t SoftSerial::write(uint8_t c)
{
	softuart_putchar(c);
  return 1;
}
