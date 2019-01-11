#include "main.h"

volatile double current_temperature = 20.0;
volatile int required_temperature = 20;

double get_current_temperature() {
	return current_temperature;
}

void set_current_temperature(double temperature) {
	current_temperature = temperature;
}

int get_required_temperature() {
	return required_temperature;
}

void set_required_temperature(int temperature) {
	if (temperature > 10 && temperature < 40) {
		required_temperature = temperature;
	}
}

int heater_pin_base = GPIOB;
int heater_pin = GPIO_PIN_7;

void heater_adapt() {
	int heat_on = HAL_GPIO_ReadPin(heater_pin_base, heater_pin);

	if (!heat_on && current_temperature < required_temperature) {
		HAL_GPIO_WritePin(heater_pin_base, heater_pin, 1);
	}
	else if (heat_on && current_temperature > required_temperature) {
		HAL_GPIO_WritePin(heater_pin_base, heater_pin, 0);
	}
}
