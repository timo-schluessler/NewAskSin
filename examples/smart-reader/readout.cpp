#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04
#define ACK 0x06
#define NACK 0x15

#include "SoftSerial.h"
#include "Arduino.h"
#include "AS.h"
#include "readout.h"

#define FIX_PREC 1000
uint32_t kWhReadout;
uint32_t m3Readout;
bool newReadoutValue = false;
bool startNewReadout = false;

#define uart_puts_P(s___) uart_puts_p(PSTR(s___))
uint8_t uart_putc(uint8_t c);
void uart_puts (uint8_t *s);
void uart_puts_p(const char * prg_s);
uint8_t uart_getc(void);
uint8_t uart_kbhit();
void reset_parser();
void parse(uint8_t c);

void pollReadout()
{
	static enum State { STARTUP, ENTER_IDLE, IDLE, WAKEUP, WAIT_FOR_WAKEUP, FLUSH, READ_IDENT, READ_DATA_MSG, SEND_BYTES, FLUSH_SENT_BYTES, READ_BYTES, REPEATER } state = STARTUP;
	static waitTimer timer;
	static uint16_t cnt = 0;
	static uint8_t bcc = 0;
	static uint8_t baud_rate_identification = 'X';
	if (state == STARTUP) {
		kWhReadout = 0xdeadcafe;
		m3Readout = 0xaffedead;
		state = ENTER_IDLE;

		UCSR0A = 0;
		UCSR0B = 0;
		UCSR0C = 0;
		#define BAUD 300
		#include "setbaud.c.h"
		#undef BAUD
		UCSR0C = (2<<UPM00) | (2<<UCSZ00); // async, even parity, 1 stop bit, 7 bit
		UCSR0B |= (1<<TXEN0)|(1<<RXEN0);
		timer.set(1500);
	}
	if (state == ENTER_IDLE) { // wait until there is silence for 1.5 s
		if (!uart_kbhit()) {
			if (timer.done()) {
				Serial << F("Readout now idle.\n");
				state = IDLE;
			}
		} else {
			if (uart_kbhit())
				uart_getc();
			timer.set(1500);
		}	
	}
	if (state == IDLE) {
		if (uart_kbhit())
			uart_getc(); // flush uart
		if (startNewReadout && !newReadoutValue) {
			startNewReadout = false;
			state = WAKEUP;
			Serial << F("Sending wakeup NULs...\n");
			timer.set(2200);
		}
	}
	if (state == WAKEUP) {
		if (timer.done()) {
			state = WAIT_FOR_WAKEUP;
			timer.set(1600);
			Serial << F("Waiting for device to wakeup...\n");
		} else
			uart_putc('\0');
	}
	if (state == WAIT_FOR_WAKEUP) {
		if (timer.done()) {
			state = FLUSH;
			UCSR0A |= (1<<TXC0); // clear TXC0
			Serial << F("Sending request...\n");
			uart_puts_P("/?!\r\n");
		}
	}
	if (state == FLUSH) {
		if (uart_kbhit())
			uart_getc();
		else if (UCSR0A & (1<<TXC0)) {
			Serial << F("Reading ident\n");
			state = READ_IDENT;
			timer.set(1500);
			cnt = 0;
		}
	}
	if (state == READ_IDENT) {
		if (uart_kbhit()) {
			if (UCSR0A & _BV(UPE0)) { // parity error
				Serial << F("EPE");
				timer.set(0); // this will force a timeout
				uart_getc();
				pollReadout();
				return;
			}
			uint8_t c = uart_getc();
			Serial << (char)c;
			timer.set(1500);
			if (cnt++ == 4)
				baud_rate_identification = c;
			if (c == '\n') {
				Serial << F("Baud rate identification is ") << (char)baud_rate_identification << F("\n");
				timer.set(1500);
				state = READ_DATA_MSG;
				bcc = 0;
				cnt = 0;
				// theese identifiers are all for mode B. mode C uses numbers, which i don't implement due to being lazy
				if (baud_rate_identification == 'A') {
					#define BAUD 600
					#include "setbaud.c.h"
					#undef BAUD
				} else if (baud_rate_identification == 'B') {
					#define BAUD 1200
					#include "setbaud.c.h"
					#undef BAUD
				} else if (baud_rate_identification == 'C') {
					#define BAUD 2400
					#include "setbaud.c.h"
					#undef BAUD
				} else if (baud_rate_identification == 'D') {
					#define BAUD 4800
					#include "setbaud.c.h"
					#undef BAUD
				} else if (baud_rate_identification == 'E') {
					#define BAUD 9600
					#include "setbaud.c.h"
					#undef BAUD
				} else if (baud_rate_identification == 'F') {
					#define BAUD 19200
					#include "setbaud.c.h"
					#undef BAUD
				} else {
					Serial << F("Unsupported baud rate identification\n");
					state = ENTER_IDLE;
					timer.set(1500);
				}
			}
		} else if (timer.done()) {
			Serial << F("Timeout while reading identification\n");
			state = ENTER_IDLE;
			timer.set(1500);
		}
	}
	if (state == READ_DATA_MSG) {
		if (uart_kbhit()) {
			if (UCSR0A & _BV(UPE0)) { // parity error
				Serial << F("EPE");
				timer.set(0); // this will end the readout
				uart_getc();
				pollReadout();
				return;
			}
			uint8_t c = uart_getc();
			Serial << (char)c;
			timer.set(1500);

			static const char end_of_msg[] = { '!', '\r', '\n', ETX };
			if (cnt == 0) {
				if (c != STX) {
					Serial << F("E1E");
					timer.set(0); // this will end the readout
				} else {
					reset_parser();
					cnt++;
				}
			} else {
				if (cnt == 1 + sizeof(end_of_msg) / sizeof(char)) {
					if (c != bcc)
						Serial << F("BCC doesn't match\n");
					else {
						Serial << F("BCC matches\n");
						newReadoutValue = true;
					}
					#define BAUD 300
					#include "setbaud.c.h"
					#undef BAUD
					state = ENTER_IDLE;
					timer.set(200);
				} else {
					bcc ^= c;
					if (c == end_of_msg[cnt - 1])
						cnt++;
					else {
						if (cnt != 1) {
							Serial << F("E2E");
							cnt = 1;
						} else
							parse(c);
					}
				}
			}
		} else if (timer.done()) {
			Serial << F("Timeout while reading data message\n");
			#define BAUD 300
			#include "setbaud.c.h"
			#undef BAUD
			state = ENTER_IDLE;
			timer.set(1500);
		}
	}
}

enum States { PARSING_KEY, PARSING_KWH, PARSING_M3, PARSING_VALUE };
static uint8_t state = PARSING_KEY;
static uint8_t buf[64];
static uint8_t buf_i = 0;

void reset_parser()
{
	state = PARSING_KEY;
	buf_i = 0;
}
void parse(uint8_t c)
{
	if (state == PARSING_KEY) {
		if (buf_i == 0 && (c == '\r' || c == '\n'))
			; // ignore newlines
		else if (c == '(') {
			if (buf_i == 3 && memcmp_P(buf, PSTR("6.8"), buf_i) == 0)
				state = PARSING_KWH;
			else if (buf_i == 4 && memcmp_P(buf, PSTR("6.26"), buf_i) == 0)
				state = PARSING_M3;
			else
				state = PARSING_VALUE;
			buf_i = 1;
			buf[0] = c;
		} else {
			buf[buf_i] = c;
			buf_i++;
		}
	} else if (state == PARSING_VALUE) {
		if (c == ')') {
			state = PARSING_KEY;
			buf_i = 0;
		}
	} else if (state == PARSING_KWH || state == PARSING_M3) {
		buf[buf_i] = c;
		buf_i++;

		if (c == ')') {
			uint8_t * sep = (uint8_t*)memchr(buf, '.', buf_i);
			uint8_t * i;
			if (sep == NULL)
				sep = (uint8_t*)memchr(buf, '*', buf_i);
			if (sep == NULL)
				sep = buf + buf_i - 1;

			uint32_t * value;
			if (state == PARSING_KWH)
				value = &kWhReadout;
			else
				value = &m3Readout;

			*value = 0;
			if (*sep == '.') {
				uint16_t mult = FIX_PREC / 10;
				i = sep + 1;
				while (*i >= '0' && *i <= '9') {
					*value += (*i - '0') * mult;
					mult /= 10;
					i++;
				}
			}

			uint32_t mult = FIX_PREC;
			i = sep - 1;
			while (*i >= '0' && *i <= '9') {
				*value += (*i - '0') * mult;
				mult *= 10;
				i--;
			}

			state = PARSING_KEY;
			buf_i = 0;
		}
	}
}

uint8_t uart_putc(uint8_t c)
{
	while (!(UCSR0A & (1<<UDRE0)))
	{
	}                             

	UDR0 = c;
	return 0;
}

void uart_puts (uint8_t *s)
{
	while (*s) {
		uart_putc(*s);
		*s = uart_getc();
		s++;
	}
}

void uart_puts_p(const char * prg_s)
{
	char c;
	while ((c = pgm_read_byte(prg_s++)))
		uart_putc(c);
}

uint8_t uart_kbhit()
{
	return !!(UCSR0A & (1<<RXC0));
}

uint8_t uart_getc(void)
{
	while (!(UCSR0A & (1<<RXC0)))
		;
	return UDR0;
}


