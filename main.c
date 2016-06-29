#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#define WDP_32MS (1<<WDP0)
#define WDP_8S  ((1<<WDP3) | (1<<WDP0))

uint8_t sleep_interval;

uint8_t values[20] = {
	0, 90, 168, 223, 252, 255, 236, 202, 162, 122,
	86, 58, 37, 22, 12, 7, 3, 2, 1, 0
};

void setupPWM() {
	// Set Timer 0 prescaler
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

// Enable watchdog interrupt, set prescaling to 1 sec
void init_wdt(uint8_t wdp) {
	// Disable interrupts
	cli();
	// Start timed sequence
	// Set Watchdog Change Enable bit
	WDTCR |= (1<<WDCE);
	// Set new prescaler (1 sec), unset reset enable
	// enable WDT interrupt
	WDTCR = (1<<WDTIE) | wdp;
	sei();
}

void sleep(uint8_t s, uint8_t mode) {
	uint8_t i;
	sei();
	sleep_interval = 0;
	while (sleep_interval < s) {
		set_sleep_mode(mode);
		wdt_reset();
		sleep_mode();
	}
	cli();
}

void adcEnable() {
	ADMUX =
		(0 << REFS0) | // VCC as voltage reference
		(0 << ADLAR) | // 10 Bit resolution
		(1 << MUX1)  | // ADC2(PB4)
		(0 << MUX0);   // ADC2(PB4)

	ADCSRA =
		(1 << ADEN)  | // ADC enable
		(1 << ADPS2) | // set prescaler to 64, bit 2
		(1 << ADPS1) | // set prescaler to 64, bit 1
		(0 << ADPS0);  // set prescaler to 64, bit 0
}

void adcDisable() {
	ADCSRA &= ~(1<<ADEN); // remove ADC enable
}

uint16_t readADC() {
	ADCSRA |= (1 << ADSC);         // start ADC measurement.
	while (ADCSRA & (1 << ADSC) ); // wait till conversion completes

	return ADCW;
}

uint16_t readLightLevel() {
	uint16_t lightLevel;

	adcEnable();
	// dummy read
	readADC();
	// reading ADC
	lightLevel = readADC();

	adcDisable();
	return lightLevel;
}

int main() {
	int i;
	setupPWM();
	init_wdt((1<<WDP0));

	// Setup OC0B (PB1) as output
	DDRB = (1<<PB1);

	while(1) {
		if(readLightLevel() > 700) {
			for(i=0; i < 20; i++) {
				init_wdt(WDP_32MS);
				writePWM(values[i]);
				sleep(1, SLEEP_MODE_IDLE);
			}
		} else {
			init_wdt(WDP_8S);
			sleep(1, SLEEP_MODE_PWR_DOWN);
		}
	}
}

ISR(WDT_vect) {
	sleep_interval++;
	wdt_reset();
	// Re-enable WDT interrupt. Normally we wouldn't do that here,
	// But we're using this routine purely as a timeout; 
	// WDT is never used for reset
	WDTCR |= (1<<WDTIE);
}
