// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

#include "define.h"

// Semaphore from uCOS
extern OS_EVENT *PBSemaphore[];
extern int new_priority_assign[];
extern int new_born[];
extern int new_born_and_kicked[];
extern long int button_times[];
extern int new_born_mult_press[];

// Function for post semaphore when PB0 is pressed
void KEY0_Pressed() {

	INT8U return_code = OS_NO_ERR;

	return_code = OSSemPost(PBSemaphore[0]);
	alt_ucosii_check_return_code(return_code);

	button_times[0] = OSTimeGet();

	if (new_born[0] == 1) {
		new_born_mult_press[0]++;
	}
	if(new_born_mult_press[0]>1){
		new_born_and_kicked[0]=1;
	}

	//////////////////////////////////////////////

}

// Function for post semaphore when PB1 is pressed
void KEY1_Pressed() {
	INT8U return_code = OS_NO_ERR;

	return_code = OSSemPost(PBSemaphore[1]);
	alt_ucosii_check_return_code(return_code);
	/////////////////////////////////////////////////////////////
	button_times[1] = OSTimeGet();

	if (new_born[1] == 1) {
		new_born_mult_press[1]++;
	}
	if(new_born_mult_press[1]>1){
		new_born_and_kicked[1]=1;
	}
	//////////////////////////////////////////////
}

// Function for post semaphore when PB2 is pressed
void KEY2_Pressed() {
	INT8U return_code = OS_NO_ERR;

	return_code = OSSemPost(PBSemaphore[2]);
	alt_ucosii_check_return_code(return_code);
	/////////////////////////////////////////////////////
	if (new_born[2] == 1) {
		new_born_mult_press[2]++;
	}
	if(new_born_mult_press[2]>1){
		new_born_and_kicked[2]=1;
	}

	button_times[2] = OSTimeGet();
	//////////////////////////////////////////////
}

// Function for post semaphore when PB3 is pressed
void KEY3_Pressed() {
	INT8U return_code = OS_NO_ERR;

	return_code = OSSemPost(PBSemaphore[3]);
	alt_ucosii_check_return_code(return_code);
	///////////////////////////////////////////////////////
	if (new_born[3] == 1) {
		new_born_mult_press[3]++;
	}
	if(new_born_mult_press[3]>1){
		new_born_and_kicked[3]=1;
	}

	button_times[3] = OSTimeGet();

	//////////////////////////////////////////////
}
// ISR when any PB is pressed
void handle_button_interrupts() {
	OSIntEnter();

	outport(LED_GREEN_O_BASE,get_pio_edge_cap(PUSH_BUTTON_I_BASE)*get_pio_edge_cap(PUSH_BUTTON_I_BASE));
	switch (get_pio_edge_cap(PUSH_BUTTON_I_BASE)) {
	case 1:
		KEY0_Pressed();
		break;
	case 2:
		KEY1_Pressed();
		break;
	case 4:
		KEY2_Pressed();
		break;
	case 8:
		KEY3_Pressed();
		break;
	}
	set_pio_edge_cap(PUSH_BUTTON_I_BASE,0x0);

	OSIntExit();
}

// Function for initializing the ISR of the PBs
// The PBs are setup to generate interrupt on falling edge,
// and the interrupt is captured when the edge comes
void init_button_irq() {
	// Enable all 4 button interrupts
	set_pio_irq_mask(PUSH_BUTTON_I_BASE, BUTTON_INT_MASK);

	// Reset the edge capture register
	set_pio_edge_cap(PUSH_BUTTON_I_BASE, 0x0);

	// Register the interrupt handler
	alt_irq_register(PUSH_BUTTON_I_IRQ, NULL, (void*) handle_button_interrupts);
}
