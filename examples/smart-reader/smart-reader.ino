#define SER_DBG

#include "SoftSerial.h"
#include "Arduino.h"
//- load library's --------------------------------------------------------------------------------------------------------
#include <AS.h>																				// the asksin framework
#include "register.h"																		// configuration sheet
#include "readout.h"
#define SER_DBG

//waitTimer xTmr;


//- arduino functions -----------------------------------------------------------------------------------------------------
void setup() {
	// - Hardware setup ---------------------------------------
	// everything off
	// - Hardware setup ---------------------------------------
	// - everything off ---------------------------------------

	EIMSK = 0;																			// disable external interrupts
	UCSR0A = 0;
	UCSR0B = 0;
	UCSR0C = 0;
	//ADCSRA = 0;																			// ADC off
	power_all_disable();																	// and everything else
	
	//DDRB = DDRC = DDRD = 0x00;															// everything as input
	//PORTB = PORTC = PORTD = 0x00;															// pullup's off

	// todo: timer0 and SPI should enable internally
	power_timer0_enable();
	power_timer2_enable();
	power_spi_enable();																		// enable only needed functions
	power_usart0_enable();

	// enable only what is really needed

/*
	TCCR0A = 0;
	TCCR0B = 0;
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR2A = 0;
	TCCR2B = 0;
	TIMSK0 = 0;
	ADCSRA = 0;
	*/

	#ifdef SER_DBG
		dbgStart();																			// serial setup
		dbg << F("Timos Homematic Smart reader\n");
		dbg << F(LIB_VERSION_STRING);
		_delay_ms (50);																		// ...and some information
	#endif
	
	
	// - AskSin related ---------------------------------------
	hm.init();																				// init the asksin framework


	//sei();																					// enable interrupts

	
	// - user related -----------------------------------------
	#ifdef SER_DBG
		dbg << F("HMID: ") << _HEX(HMID,3) << F(", MAID: ") << _HEX(MAID,3) << F("\n\n");		// some debug
	#endif
	
}


void serialEvent();
void loop() {
	// - AskSin related ---------------------------------------
	hm.poll();																				// poll the homematic main loop
	serialEvent();
	pollReadout();
	
	if (PIND & _BV(PD0)) // display RXD on led
		setPinLow(LED_RED_PORT, LED_RED_PIN);
	else
		setPinHigh(LED_RED_PORT, LED_RED_PIN);
	// - user related -----------------------------------------

}


//- predefined functions --------------------------------------------------------------------------------------------------
void serialEvent() {
	#ifdef SER_DBG
	
	static uint8_t i = 0;																	// it is a high byte next time
	while (Serial.available()) {
		uint8_t inChar = (uint8_t)Serial.read();											// read a byte
		if (inChar == '\n') {																// send to receive routine
			i = 0;
			hm.sn.active = 1;
		}
		
		if      ((inChar>96) && (inChar<103)) inChar-=87;									// a - f
		else if ((inChar>64) && (inChar<71))  inChar-=55;									// A - F
		else if ((inChar>47) && (inChar<58))  inChar-=48;									// 0 - 9
		else continue;
		
		if (i % 2 == 0) hm.sn.buf[i/2] = inChar << 4;										// high byte
		else hm.sn.buf[i/2] |= inChar;														// low byte
		
		i++;
	}
	#endif
}


