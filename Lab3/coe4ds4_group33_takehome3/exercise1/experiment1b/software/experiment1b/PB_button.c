// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

#include "define.h"

//not using KEY_0

void KEY1_Pressed(elevator_struct *elevator) {//when PB 1 pressed, load switches 16&& 15 for door open time
	int sw1516 = (IORD(SWITCH_I_BASE, 0) >> 15) & 0x3;


	switch ((IORD(SWITCH_I_BASE, 0) >> 15) & 0x3) {
	case 0:
		elevator->door_open_time = 500; //all in milliseconds
	case 1:
		elevator->door_open_time = 1000;
	case 2:
		elevator->door_open_time = 1500;
	case 3:
		elevator->door_open_time = 2000;
		//printf("PB 1 pressed");

	}
	if ((((IORD(PUSH_BUTTON_I_BASE, 0)) >> 1) & 0x1) == 1) { //active low so when detects 1, means let go of
		//why does this work and above doesnt wft?
		//printf("The value of switch 16 15 was %x\n", sw1516);
		if(sw1516 == 0)elevator->door_open_time = 500; //all in milliseconds
		if(sw1516 == 1)elevator->door_open_time = 1000;
		if(sw1516 == 2)elevator->door_open_time = 1500;
		if(sw1516 == 3)elevator->door_open_time = 2000;
		printf("Update the door open time to %d ms\n", elevator->door_open_time);
		load_counter_0_config(sw1516);
	}

}

void KEY2_Pressed(elevator_struct *elevator) {//when PB 2 pressed, load switches 16 && 15 for time b/w floors
	int sw1516=(IORD(SWITCH_I_BASE, 0)>>15)&0x3;
	if ((((IORD(PUSH_BUTTON_I_BASE, 0)) >> 2) & 0x1) == 1) { //active low so when detects 1, means let go of
		//why does this work and above doesnt wtf?
		//printf("The value of switch 16 15 was %x\n", sw1516);
		if(sw1516 == 0)elevator->floor_travel_time= 500; //all in milliseconds
		if(sw1516 == 1)elevator->floor_travel_time = 1000;
		if(sw1516 == 2)elevator->floor_travel_time = 1500;
		if(sw1516 == 3)elevator->floor_travel_time = 2000;
		printf("Update the floor_travel_time to %d ms\n", elevator->floor_travel_time);
		load_counter_1_config(sw1516);
	}

//	if ((((IORD(PUSH_BUTTON_I_BASE, 0)) >> 2) & 0x1) == 1) { //active low so when detects 1, means let go of
//		printf("PB 2 pressed, update the inter-floor travel time to %d\n", elevator->floor_travel_time);
//		load_counter_1_config(elevator->floor_travel_time);
//	}
}

void KEY3_Pressed(elevator_struct *elevator) {//when PB 3 pressed, KEEP DOOR OPEN, IF RELEASED, CAN CLOSE DOOR
	// GET DATA BIT FOR BUTTON[3]
	int button = ((IORD(PUSH_BUTTON_I_BASE, 0)) >> 3) & 0x1;
	//printf("The button 3 bit is %d", button);
	//printf("Button 3 pressed\n");
	if (button != 1) {
		printf("dont close button is pressed!\n");
		elevator->keep_door_open = 1;
	}
	if (button != 0) {
		printf("dont close button has been let go\n");
		elevator->keep_door_open = 0;
	}
}

// ISR when any PB is pressed
void handle_button_interrupts(elevator_struct *elevator) {
	IOWR(LED_GREEN_O_BASE, 0, IORD(PUSH_BUTTON_I_BASE, 3)*IORD(PUSH_BUTTON_I_BASE, 3));
	//printf("Button %d pressed\n",(IORD(PUSH_BUTTON_I_BASE, 3)) );
	switch (IORD(PUSH_BUTTON_I_BASE, 3)) {
	//case 1: KEY0_Pressed(); break;
	case 2:
		KEY1_Pressed(elevator);
		break;
	case 4:
		KEY2_Pressed(elevator);
		break;
	case 8:
		KEY3_Pressed(elevator);
		break;
	}
	IOWR(PUSH_BUTTON_I_BASE, 3, 0x0);
}

// Function for initializing the ISR of the PBs
// The PBs are setup to generate interrupt on falling edge,
// and the interrupt is captured when the edge comes
void init_button_irq(elevator_struct *elevator) {
	// Enable all 4 button interrupts
	IOWR(PUSH_BUTTON_I_BASE, 2, BUTTON_INT_MASK);

	// Reset the edge capture register
	IOWR(PUSH_BUTTON_I_BASE, 3, 0x0);


	// Register the interrupt handler by detecting that all keys are off

	alt_irq_register(PUSH_BUTTON_I_IRQ, (void*) elevator, (void*) handle_button_interrupts);

}
