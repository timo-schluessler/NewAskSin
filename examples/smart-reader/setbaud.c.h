#include <util/setbaud.h>
UBRR0H = UBRRH_VALUE;
UBRR0L = UBRRL_VALUE;
#if USE_2X
UCSR0A |= (1 << U2X0);
#else
UCSR0A &= ~(1 << U2X0);
#endif
