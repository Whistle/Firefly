#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

void setupPWM() {
	// Set Timer 0 prescaler to clock/8.
	TCCR0B |= (1 << CS01) | (1 << CS00);

	// Set to 'Fast PWM' mode
	TCCR0A |= (1 << WGM01) | (1 << WGM00);

	// Clear OC0B (PB1) output on compare match, upwards counting.
	TCCR0A |= (1 << COM0B1);

	// Set duty cycle to 0
	OCR0B = 0;
}

void writePWM (uint8_t val) {
	OCR0B = val;
}

uint8_t values[10] = {0, 20, 50, 70, 90, 110, 160, 190, 210, 255};

int main() {
	DDRB = (1<<PB1);
	wdt_enable(WDTO_8S);
	while(1);
}

ISR(WDT_vect) {

}
