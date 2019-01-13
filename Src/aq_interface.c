#include "main.h"
#include "lcd5110.h"

static void on_number();
static void on_choice();
static void on_click();

LCD5110_display interface_lcd;

volatile int output_pins[] = {GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11};
volatile int input_pins[] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
volatile int diodes[] = {GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15};

volatile uint32_t last_tick;

enum {
  button_a,
  button_b,
  button_c,
  button_d,
  button_star,
  button_hash
};
enum {
  start_menu,
  input_menu
};

volatile int current_menu = start_menu;
volatile int current_number_input = 0;
volatile int light_state = 0;

void interface_output_pins_write(int value) {
	for (int i = 0; i < 4; i++) {
		HAL_GPIO_WritePin(GPIOD, output_pins[i], value);
	}
}


void interface_init() {
	last_tick = HAL_GetTick();

	interface_lcd.hw_conf.spi_handle = &hspi2;
	interface_lcd.hw_conf.spi_cs_pin =  LCD1_CS_Pin;
	interface_lcd.hw_conf.spi_cs_port = LCD1_CS_GPIO_Port;
	interface_lcd.hw_conf.rst_pin =  LCD1_RST_Pin;
	interface_lcd.hw_conf.rst_port = LCD1_RST_GPIO_Port;
	interface_lcd.hw_conf.dc_pin =  LCD1_DC_Pin;
	interface_lcd.hw_conf.dc_port = LCD1_DC_GPIO_Port;
	interface_lcd.def_scr = lcd5110_def_scr;
	LCD5110_init(&interface_lcd.hw_conf, LCD5110_NORMAL_MODE, 0x40, 2, 3);

	interface_output_pins_write(1);
}

static void on_click(int col_pin, int row_pin) {
  // 1 - 9
  if (col_pin < 3 && row_pin < 3) {
	  on_number(1 + row_pin * 3 + col_pin);
  }
  // 0
  else if (col_pin == 1 && row_pin == 3) {
	  on_number(0);
  }
  // a - d
  else if (col_pin == 3) {
	  on_choice(button_a + row_pin);
  }
  // * #
  else if (col_pin == 0 && row_pin == 3) {
	  on_choice(button_star);
  }
  else if (col_pin == 2 && row_pin == 3) {
	  on_choice(button_hash);
  }

  interface_display();
}

static void on_number(int number) {
	if (current_menu == input_menu){
		current_number_input = current_number_input * 10 + number;
	}
}

static void on_choice(int choice) {
  if (current_menu == start_menu) {
	  if (choice == button_a) {
		  HAL_GPIO_WritePin(GPIOD, diodes[0], 1);
		  current_menu = input_menu;
	  }
	  if (choice == button_c) {
		  dispancer_feed();
	  }
  }
  else if (current_menu == input_menu) {
	  if (choice == button_a) {
		  HAL_GPIO_WritePin(GPIOD, diodes[0], 0);
		  set_required_temperature(current_number_input);
		  current_menu = start_menu;
		  current_number_input = 0;
	  }
	  else if (choice == button_c) {
		  HAL_GPIO_WritePin(GPIOD, diodes[0], 0);
		  current_menu = start_menu;
		  current_number_input = 0;
	  }
	  else if (choice == button_d) {
		  current_number_input /= 10;
	  }
  }

  if (choice == button_b) {
	  light_state = !light_state;
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, light_state);
  }
}

void print_text(char* text) {
	LCD5110_clear_scr(&interface_lcd);
	LCD5110_set_cursor(0, 0, &interface_lcd);
	LCD5110_print(text, BLACK, &interface_lcd);
}

void interface_display() {
	char text[100];
	if (current_menu == start_menu) {
		char temperature[100];
		sprintf(temperature, "%.2f/%d", get_current_temperature(), get_required_temperature());
		sprintf(text, "Hello\nt:%11s\nA - set temp\nB - light %s\nC - feed\n", temperature, light_state ? "off" : "on");
	}
	else if (current_menu == input_menu) {
		sprintf(text, "%13d\nA - accept\nC - cancel\nD - delete\n", current_number_input);
	}

	print_text(text);
}

void interface_on_input() {
	// DEPRECATED
	uint32_t current_tick = HAL_GetTick();

	if (current_tick - last_tick > 175) {
		int col_pin = -1;
		int row_pin = -1;

		interface_output_pins_write(0);

		for (int output_pin = 0; output_pin < 4; output_pin ++) {
			HAL_GPIO_WritePin(GPIOD, output_pins[output_pin], 1);

			for (int input_pin = 0; input_pin < 4; input_pin ++) {
				if (HAL_GPIO_ReadPin(GPIOD, input_pins[input_pin])) {
					row_pin = output_pin;
					col_pin = input_pin;
					break;
				}
			}
			HAL_GPIO_WritePin(GPIOD, output_pins[output_pin], 0);
		}

		if (row_pin != -1 && col_pin != -1) {
			on_click(col_pin, row_pin);
		}

		last_tick = current_tick;
		interface_output_pins_write(1);
	}
}
