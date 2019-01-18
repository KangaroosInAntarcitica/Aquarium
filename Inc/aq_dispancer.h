#ifndef AQ_DISPANCER_H_
#define AQ_DISPANCER_H_

typedef enum {
	closed,
	open
} dispenser_state;

void dispenser_init();
// feed - Open the dispenser and close it after some time
void dispenser_feed();
void dispenser_timer_callback();

// get state - whether it is open or closed
dispenser_state dispenser_get_state();


#endif /* AQ_DISPANCER_H_ */
