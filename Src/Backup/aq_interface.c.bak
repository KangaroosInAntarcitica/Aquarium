#include "main.h"
#include "lcd5110.h"
#include "keyboard.h"

LCD5110_display interface_lcd;

typedef enum {
  start_menu,
  input_menu
} menu;

// Current menu
volatile menu current_menu = start_menu;
// When in input_menu - current number, that is being typed
volatile int current_number_input = 0;
// Whether light for display is turned on or off
volatile int light_state = 0;

void print_text(char* text) {
	// Auxiliary function, prints text on display
	LCD5110_set_cursor(0, 0, &interface_lcd);
	// LCD5110_clear_scr(&interface_lcd);
	LCD5110_print(text, BLACK, &interface_lcd);
}

void interface_display() {
	// Displays all the required data on the display
	char text[100];
	if (current_menu == start_menu) {
		char temperature[100];
		sprintf(temperature, "%.2f/%d", get_current_temperature(), get_required_temperature());
		sprintf(text,
				"Hello        \n"
				"t:%11s\n"
				"A - set temp \n"
				"B - light %s\n"
				"C - feed     \n",
				temperature,
				light_state ? "off" : "on ");
	}
	else if (current_menu == input_menu) {
		sprintf(text,
				"%13d\n"
				"A - accept   \n"
				"C - cancel   \n"
				"D - delete   \n"
				"             \n",
				current_number_input);
	}

	print_text(text);
}

// For keyboard
static void on_number(int);
static void on_choice(button);

keyboard board;
uint16_t output_pins[] = {GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11};
uint16_t input_pins[] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
volatile uint16_t diodes[] = {GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15};

void interface_init() {
	interface_lcd.hw_conf.spi_handle = &hspi2;
	interface_lcd.hw_conf.spi_cs_pin =  LCD1_CS_Pin;
	interface_lcd.hw_conf.spi_cs_port = LCD1_CS_GPIO_Port;
	interface_lcd.hw_conf.rst_pin =  LCD1_RST_Pin;
	interface_lcd.hw_conf.rst_port = LCD1_RST_GPIO_Port;
	interface_lcd.hw_conf.dc_pin =  LCD1_DC_Pin;
	interface_lcd.hw_conf.dc_port = LCD1_DC_GPIO_Port;
	interface_lcd.def_scr = lcd5110_def_scr;
	LCD5110_init(&interface_lcd.hw_conf, LCD5110_NORMAL_MODE, 0x40, 2, 3);

	keyboard new_board = {
		GPIOD, GPIOD, input_pins, output_pins, *on_number, *on_choice
	};
	board = new_board;
	keyboard_init(&board);
}

static void on_number(int number) {
	// Called when a number is pressed
	if (current_menu == input_menu){
		// Add a new least-significant digit
		current_number_input = current_number_input * 10 + number;
	}
	interface_display();
}

static void on_choice(button choice) {
  // Called when a non-number button is pressed
  if (current_menu == start_menu) {
	  if (choice == button_a) {
		  // Input temperature
		  current_menu = input_menu;
	  }
	  if (choice == button_c) {
		  // Feed the fish
		  dispenser_feed();
	  }
  }
  else if (current_menu == input_menu) {
	  if (choice == button_a) {
		  // A - Accept - get inputed required temperature menu, return to main menu
		  set_required_temperature(current_number_input);
		  current_menu = start_menu;
		  current_number_input = 0;
	  }
	  else if (choice == button_c) {
		  // C - Cancel - return to main menu
		  current_menu = start_menu;
		  current_number_input = 0;
	  }
	  else if (choice == button_d) {
		  // Delete the least significant digit
		  current_number_input /= 10;
	  }
  }

  if (choice == button_b) {
	  // B - Bright - toggle light
	  light_state = !light_state;
	  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, light_state);
  }

  interface_display();
}

void interface_on_input() {
	// pass the interrupt to keyboard
	keyboard_on_input(&board);
}
