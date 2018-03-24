// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

#include "define.h"


int main()
{
	int i;
	elevator_struct elevator;

	elevator.motion = 0;
	elevator.current_floor = 0;
	elevator.direction = 1;
	elevator.next_floor = 0;
	for(i=0;i<12;i++){
		elevator.request_queue[i] = 0;
	}
	elevator.door_close_ready = 0;
	elevator.keep_door_open = 0;
	elevator.door_open_time = 0;
	elevator.arrived_at_floor = 0;
	elevator.floor_travel_time = 0;
	elevator.counter_1_running = 0;

	printf("Start main...\n");

	init_button_irq(&elevator);
	printf("PB initialized...\n");

	init_counter_0_irq(&elevator);
	printf("Counter IRQ 0 initialized...\n");

	init_counter_1_irq(&elevator);
	printf("Counter IRQ 1 initialized...\n");


	init_switches_irq(&elevator);
	printf("Switches IRQ initialized...\n");



	printf("\n-----------------------------------------------------------------\n------The ITB (Year 2030) elevator simulator will now begin------\n-----------------------------------------------------------------\n");

	//	IOWR(LED_GREEN_O_BASE, 0, 0x0);
	//IOWR(LED_RED_O_BASE, 0, 0x0);

	//printf("Switch value: %X\n", IORD(SWITCH_I_BASE, 0));


	//printf("The address location of Seven Segment Peripheral is: %x\n", SEVEN_SEGMENT_N_O_0_BASE);


	elevator_sim(&elevator);

}
