#include "./util.h"
#include <stdint.h>

bool Debounced::loop(bool currently_down) {
	uint32_t now = millis();
	if (currently_down) {
		press_last_detected = now;
		if (!pressing) {
			// Start of new button press
			pressing = true;
			press_count++;
			return true;
		}
	} else {
		if (pressing && now > press_last_detected + debounce) {
			pressing = false;
		}
	}
	return false;
}

uint16_t Debounced::get_and_reset_press_count() {
	uint16_t output = press_count;
	press_count = 0;
	return output;
}