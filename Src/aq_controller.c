#include "main.h"

volatile double current_temperature = 20.5;
volatile int required_temperature = 20;

// Functions to get and set temperature / current temperature
// Are used to make the interaction safer
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
	// Check - temperatures higher than 40 are probably a mistake
	if (temperature > 10 && temperature < 40) {
		required_temperature = temperature;
	}
}


// THERMOMETER FUNCTIONALITY
TM_OneWire_t oneWire;
const int ONEWIRE_TRANSLATE_TEMPERATURE = 0x44;

void temperature_init() {
	// One Wire
	// Init system clock for maximum system speed
	TM_RCC_InitSystem();
	TM_OneWire_Init(&oneWire, GPIOB, GPIO_PIN_4);
}

void temperature_measure() {
	int result = 1;

	// Reset - start interactions (like start bit)
	// Skip Rom - command selects all the slaves on line
	// TranslateTemperature - command asks thermometer to measure the temperature
	// While the measurement is taken, thermometer sends 0 bit - signals
	result &= TM_OneWire_Reset(&oneWire);
	TM_OneWire_WriteByte(&oneWire, ONEWIRE_CMD_SKIPROM);
	TM_OneWire_WriteByte(&oneWire, ONEWIRE_TRANSLATE_TEMPERATURE);
	int reading;
	while ((reading = TM_OneWire_ReadByte(&oneWire)) == 0){};

	// ReadScratchpad - command to read the internal contents of the thermometer buffer
	// The first 2 bytes: Lower byte, Higher byte. Data will be real temperature * 16
	result &= TM_OneWire_Reset(&oneWire);
	TM_OneWire_WriteByte(&oneWire, ONEWIRE_CMD_SKIPROM);
	TM_OneWire_WriteByte(&oneWire, ONEWIRE_CMD_RSCRATCHPAD);
	uint16_t temperature = TM_OneWire_ReadByte(&oneWire) + TM_OneWire_ReadByte(&oneWire) * 0x100;

	// 0xFFFF (max value) - termometer did not react
	if (temperature != 0xFFFF) {
		set_current_temperature((double) temperature / 16);
	}
}

// HEATER - HEATING SYSTEM

GPIO_TypeDef* heater_port = GPIOB;
int heater_pin = GPIO_PIN_7;

int heater_is_on() {
	// returns whether heater is currently heating
	return HAL_GPIO_ReadPin(heater_port, heater_pin);
}

void heater_adapt() {
	// checks whether heater needs to be on and reacts correspondingly
	int heat_on = HAL_GPIO_ReadPin(heater_port, heater_pin);

	if (!heat_on && current_temperature < required_temperature) {
		HAL_GPIO_WritePin(heater_port, heater_pin, 1);
	}
	else if (heat_on && current_temperature > required_temperature) {
		HAL_GPIO_WritePin(heater_port, heater_pin, 0);
	}
}

