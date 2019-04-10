#include "main.h"

// Open and close rotation from 0 to 1
const float ROTATION_OPEN = 0.4;
const float ROTATION_CLOSED = 0.866;

// Time we wait
uint16_t wait_time = 500;
volatile uint16_t open_tick;

// The main command is dispenser_feed(). It sets the state to open.
// Because it would cost too much to wait in the function, it is
// checked whether the dispenser needs to be closed in the timer-callback.

volatile dispenser_state current_state;
static void dispenser_set_state(dispenser_state state) {
	// Changes the state
	if (current_state == state)
		return;
	current_state = state;

	// find what the rotation is
	float rotation;
	if (state == closed) rotation = ROTATION_CLOSED;
	else rotation = ROTATION_OPEN;

	TIM_HandleTypeDef htim10;

	// Send command to the servo - using PWM
	htim10.Instance = TIM10;
	TIM10->PSC = SystemCoreClock / 1e6 - 1;
	TIM10->CCR1 = 2500 - (int) (125 * (1 + rotation));

	HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
}

dispenser_state dispenser_get_state() {
	return current_state;
}

static void dispenser_set_state(dispenser_state state);

void dispenser_init() {
	dispenser_set_state(closed);
}

void dispenser_timer_callback() {
	// set state to closed if some time has elapsed
	uint16_t current_tick = HAL_GetTick();

	if (current_tick - open_tick > wait_time)
		dispenser_set_state(closed);
}


void dispenser_feed() {
	// set state to open
	uint16_t current_tick = HAL_GetTick();

	if (current_tick - open_tick > wait_time){
		open_tick = current_tick;
		dispenser_set_state(open);
	}
}

