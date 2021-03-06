#ifndef AQ_CONTROLLER_H_
#define AQ_CONTROLLER_H_

double get_current_temperature();
void set_current_temperature(double temperature);

int get_required_temperature();
void set_required_temperature(int temperature);

// THERMOMETER FUNCTIONS
// received data can be got with the get_current_temperature() function
void temperature_init();
void temperature_measure();

// HEATER FUNCTIONS
int heater_is_on();
void heater_adapt();

#endif /* AQ_CONTROLLER_H_ */
