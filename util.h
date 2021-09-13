#include <stdint.h>
#include <Arduino.h>
enum lock_status {
  closed    = 0,
  open      = 1,
  inbetween = 2
};

// Taken and modified from https://github.com/ZeusWPI/OBUS
class Debounced {
	private:
		uint16_t press_count;
		bool pressing;
		uint32_t press_last_detected;
		uint32_t debounce;
	public:
		Debounced(uint32_t debounce_time) :
			press_count(0),
			pressing(false),
			press_last_detected(0),
			debounce(debounce_time)
			{};
		Debounced() : Debounced(50) {};

		bool loop(bool currently_down);
		uint16_t get_and_reset_press_count();
};
