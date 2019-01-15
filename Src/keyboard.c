#include "stm32f4xx_hal.h"
#include "keyboard.h"

static void keyboard_pins_write(keyboard* board, int value) {
	// write to all output pins a certain value
	for (int i = 0; i < 4; i++) {
		HAL_GPIO_WritePin(board->output_pins_port, board->output_pins[i], value);
	}
}

static void keyboard_on_press(keyboard* board, int col_pin, int row_pin) {
  // Function is called on any button pressed.
  // Finds out what button is pressed and calls corresponding function
  // 1 - 9
  void (*on_number)(int) = board->on_number;
  void (*on_choice)(button) = board->on_choice;

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
}

// Initialize the keyboard
void keyboard_init(keyboard* board) {
	// We put on - so we can later receive the interrupt
	keyboard_pins_write(board, 1);
	board->last_tick = HAL_GetTick();
}

// User should call this function manually - when interrupt from any pin comes
void keyboard_on_input(keyboard* board) {
	uint32_t current_tick = HAL_GetTick();

	if (current_tick - board->last_tick > 175) {
		int col_pin = -1;
		int row_pin = -1;

		// get col_pin
		// loop through all input pins and find which of them is pressed
		for (int input_pin = 0; input_pin < 4; ++input_pin) {
			if (HAL_GPIO_ReadPin(board->input_pins_port, board->input_pins[input_pin])) {
				col_pin = input_pin;
				break;
			}
		}

		// outputs off
		keyboard_pins_write(board, 0);

		if (col_pin != -1) {
			// get row_pin
			// Turn on the outputs one by one and get when the input works
			for (int output_pin = 0; output_pin < 4; ++output_pin) {
				HAL_GPIO_WritePin(board->output_pins_port, board->output_pins[output_pin], 1);
				if (HAL_GPIO_ReadPin(board->input_pins_port, board->input_pins[col_pin])) {
					row_pin = output_pin;
					break;
				}
			}
		}

		// Call on_press if some button was pressed
		if (row_pin != -1 && col_pin != -1) {
			keyboard_on_press(board, col_pin, row_pin);
		}

		board->last_tick = current_tick;
		keyboard_pins_write(board, 1);
	}
}
