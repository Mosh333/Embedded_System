////////////////////////
// CODE SECTION BEGIN //
////////////////////////

#include "define.h"

extern volatile int request_queue;

//************************Need to figure out logic to implement real time update of 7 Seg based on custom counter************************************//

void switch_interrupt(elevator_struct *elevator){
	int i=0;
	int sw= IORD(SWITCH_I_BASE, 3)& 0xFFF;
	int rleds=0;
	for(i=0;i<=11;i++){
		if((sw>>i)&0x1)break;
	}
	elevator->request_queue[i]=1;



	rleds=0;
	for (i = 0; i <= 11; i++) {
		if (elevator->request_queue[i] == 1) {
			rleds = rleds | (0x1 << i);
		}
	}//figure out the floors that have requests
	IOWR(LED_RED_O_BASE, 0, rleds);
	//***Remove*******************************************************************************//
	//elevator->current_floor = i;
	//IOWR(SEVEN_SEGMENT_N_O_0_BASE, 0, disp_seven_seg(elevator->current_floor));
	//****************************************************************************************//
	//printf("\n-----In IRQ-----\nSwitch %d toggled\n", i);
	//for(i=11;i>=0;i--){
	//	printf("%d",elevator->request_queue[i]);
	//}

	//printf("\n-----Out IRQ------\n", i);

	IOWR(SWITCH_I_BASE, 3, 0x0);


	//printf(" switch %d toggled\n", sw);
//
//	for(i=0;i<=11;i++){
//		if(elevator->request_queue[i]==1){rleds=rleds|(0x1<<i);}
//	}



	//IOWR(LED_RED_O_BASE, 0, rleds);
	//return;


}



// Function for initializing the ISR of the Counter
void init_switches_irq(elevator_struct *elevator) {
	IOWR(SWITCH_I_BASE, 3, 0x0); //check this, do we need it?
	IOWR(SWITCH_I_BASE, 2, 0xFFF);//unmask the lowest 12 switches (0-11)
	alt_irq_register(SWITCH_I_IRQ, (void*)elevator, (void*)switch_interrupt );
}

////////////////////////
// CODE SECTION END   //
////////////////////////