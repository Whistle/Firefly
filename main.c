#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

int main() {
	wdt_enable(WDTO_8S);
	while(1);
}

ISR(WDT_vect) {

}
