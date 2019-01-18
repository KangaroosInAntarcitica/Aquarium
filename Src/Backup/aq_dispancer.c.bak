#include "main.h"

static void dispancer_set_state(float state);

// Open and close rotation from 0 to 1
const float OPEN = 0.5;
const float CLOSE = 0.0;

// Time we wait
uint16_t wait_time = 500;
volatile uint16_t open_tick;

void dispancer_init() {
	dispancer_set_state(CLOSE);
}

void dispancer_timer_callback() {
	uint16_t current_tick = HAL_GetTick();

	if (current_tick - open_tick > wait_time)
		dispancer_set_state(CLOSE);
}

volatile float current_state;
static void dispancer_set_state(float state) {
	if (current_state == state)
		return;

	current_state = state;

	TIM_HandleTypeDef htim10;

	htim10.Instance = TIM10;
	TIM10->PSC = SystemCoreClock / 1e6 - 1;
	TIM10->CCR1 = 2500 - (int) (125 * (1 + state));

	HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
}


void dispancer_feed() {
	uint16_t current_tick = HAL_GetTick();

	if (current_tick - open_tick > wait_time){
		open_tick = current_tick;
		dispancer_set_state(OPEN);
	}
}

