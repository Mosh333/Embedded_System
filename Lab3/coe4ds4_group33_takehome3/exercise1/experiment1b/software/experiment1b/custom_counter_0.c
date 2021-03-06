////////////////////////
// CODE SECTION BEGIN //
////////////////////////

#include "define.h"
//custom counter 0 is for controlling the door open/close

// ISR when the counter is expired
void handle_counter_0_expire_interrupts(elevator_struct *elevator)
{
	//printf("\nThe doors may be closed if the hold open button is not held  \n");
	elevator->door_close_ready=1;
	//printf("elevator->door_close_ready= %d  \n", elevator->door_close_ready);

	IOWR(CUSTOM_COUNTER_COMPONENT_0_BASE, 2, 0);
}

void reset_counter_0(elevator_struct *elevator) {
	//printf("Resetting counter 0 value\n");

	elevator->door_close_ready=0;

	//printf("elevator->door_close_ready= %d  \n", elevator->door_close_ready);

	IOWR(CUSTOM_COUNTER_COMPONENT_0_BASE, 1, 1);
	IOWR(CUSTOM_COUNTER_COMPONENT_0_BASE, 1, 0);

	IOWR(CUSTOM_COUNTER_COMPONENT_0_BASE, 2, 0);
}

int read_counter_0(){
	return IORD(CUSTOM_COUNTER_COMPONENT_0_BASE, 0);
}

int read_counter_0_interrupt() {
	return IORD(CUSTOM_COUNTER_COMPONENT_0_BASE, 2);
}

void load_counter_0_config(int config) {
	//printf("Loading counter 0 config %d\n", config);

	IOWR(CUSTOM_COUNTER_COMPONENT_0_BASE, 3, config);
}

// Function for initializing the ISR of the Counter
void init_counter_0_irq(elevator_struct *elevator) {
	IOWR(CUSTOM_COUNTER_COMPONENT_0_BASE, 2, 0);

	alt_irq_register(CUSTOM_COUNTER_COMPONENT_0_IRQ, (void*)elevator , (void*)handle_counter_0_expire_interrupts );
}

////////////////////////
// CODE SECTION END   //
////////////////////////
