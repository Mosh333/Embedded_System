// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

// This is the embedded software for the
// LCD / Camera design

#include <stdio.h>
#include "io.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "sys/alt_stdio.h"
#include "priv/alt_busy_sleep.h"

void TouchPanel_int(void) {
	static int exposure = 0x0400, run = 1;
	static int config = 0;
	static int new_config = 4;
	int TP_val, x_val, y_val, key = 6;
	int coef_set_x[3]; //static bc need to keep value through function
	//invocations
	static int coeff_config = 0;

	TP_val = IORD(NIOS_LCD_CAMERA_COMPONENT_0_TOUCHPANEL_BASE, 0);
	x_val = (TP_val >> 20) & 0xFF;
	y_val = (TP_val >> 4) & 0xFF;

	if (((TP_val >> 31) & 0x1) && (x_val >= 0xC9) && (x_val <= 0xF1)) {
		if ((y_val >= 0x17) && (y_val <= 0x33)) { // Key 0
			key = 0;
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CONSOLE_BASE, 0, 0x1);
		}
		if ((y_val >= 0x3D) && (y_val <= 0x58)) { // Key 1
			key = 1;
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CONSOLE_BASE, 0, 0x2);
		}
		if ((y_val >= 0x62) && (y_val <= 0x7E)) { // Key 2
			key = 2;
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CONSOLE_BASE, 0, 0x4);
		}
		if ((y_val >= 0x88) && (y_val <= 0xA4)) { // Key 3
			key = 3;
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CONSOLE_BASE, 0, 0x8);
		}
		if ((y_val >= 0xAE) && (y_val <= 0xC9)) { // Key 4
			key = 4;
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CONSOLE_BASE, 0, 0x10);
		}
		if ((y_val >= 0xD3) && (y_val <= 0xEF)) { // Key 5
			key = 5;
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CONSOLE_BASE, 0, 0x20);
		}
	}
	else IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CONSOLE_BASE, 0, 0x0);

	if (IORD(NIOS_LCD_CAMERA_COMPONENT_0_TOUCHPANEL_BASE, 2) & 0x2) { // posedge
		switch (key) {
		case 0:
			if (run == 1) {
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CAMERA_BASE, 1, 0x8);
				run = 0;
			} else {
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CAMERA_BASE, 1, 0x4);
				run = 1;
			}
			break;
		case 1:
			config++;
			if (config == 4)
				config = 0;
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, config);
			break;
		case 2:
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, 4);
			break;

		case 3:
			coeff_config++; //static variable to hold value
			if (coeff_config == 6){
				coeff_config = 0;
			}

			//IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, 5);//use 5th filter config in the .v

			switch (coeff_config) {

			//32 bits worth of values passed @ every offset
			//4, 4, and then 2 values passed, 8 bits each 8+8+8+8=32

			//-1 FF
			//-2 FE
			//0  0
			//-6 FA
			//1  1
			//2  2
			//6  6

			//40 28
			//48 30
			//72 48
			case 0:
				coef_set_x[0]= 0xFFFEFF00;//-1, -2, -1, 0 ==32 BITS
				coef_set_x[1]= 0x00000102;// 0, 0, 1, 2
				coef_set_x[2]= 0x00000128; //1, 40
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, 5);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 6, coef_set_x[0]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 7, coef_set_x[1]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 8, coef_set_x[2]);
				//printf("This is 0");

				break;


			case 1:
				coef_set_x[0]= 0xFF0001FE;//-1, 0, 1, -2 ==32 BITS
				coef_set_x[1]= 0x0002FF00;//0, 2, -1, 0
				coef_set_x[2]= 0x00000128; //1, 40
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, 5);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 6, coef_set_x[0]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 7, coef_set_x[1]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 8, coef_set_x[2]);

				//printf("This is 1");
				break;
			case 2:
				coef_set_x[0]= 0xFEFEFE00;//-2, -2, -2, 0 ==32 BITS
				coef_set_x[1]= 0x00000202;// 0, 0, 2, 2
				coef_set_x[2]= 0x00000230; //2, 48
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, 5);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 6, coef_set_x[0]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 7, coef_set_x[1]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 8, coef_set_x[2]);
				//printf("This is 2");
				break;
			case 3:
				coef_set_x[0]= 0xFE0002FE;//-2,0, 2, -2 ==32 BITS
				coef_set_x[1]= 0x0002FE00;// 0, 2, -2, 0
				coef_set_x[2]= 0x00000230; //2, 48
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, 5);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 6, coef_set_x[0]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 7, coef_set_x[1]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 8, coef_set_x[2]);
				//printf("This is 3");
				break;
			case 4:
				coef_set_x[0]= 0xFEFAFE00;//-2,-6,-2,0 ==32 BITS
				coef_set_x[1]= 0x00000206;// 0, 0, 2,6
				coef_set_x[2]= 0x00000248; //2,72
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, 5);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 6, coef_set_x[0]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 7, coef_set_x[1]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 8, coef_set_x[2]);
				//printf("This is 4");
				break;
			case 5:
				coef_set_x[0]= 0xFE0002FA;//-2,0,2,-6 ==32 BITS
				coef_set_x[1]= 0x0006FE00;// 0, 6,-2,0
				coef_set_x[2]= 0x00000248; //2,72
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, 5);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 6, coef_set_x[0]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 7, coef_set_x[1]);
				IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 8, coef_set_x[2]);
				//printf("This is 5");
				break;
			}

			printf("Key 3 has been pressed!!\n");
			printf("We are loading a 2D Edge filter set %d!!\n\n", coeff_config);


			//IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, 5); //filter_config = 5 is for the exp 3 filter!!!!!
			//IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, defineOwnAddressHere, coef_set_x);

			break;
		case 4:
			if (exposure <= 0xFEFF)
				exposure += 0x0100;
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CAMERA_BASE, 0, exposure);
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CAMERA_BASE, 1, 0x2);
			break;
		case 5:
			if (exposure >= 0x0100)
				exposure -= 0x0100;
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CAMERA_BASE, 0, exposure);
			IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CAMERA_BASE, 1, 0x2);
			break;
		}
	}

	TP_val = IORD(NIOS_LCD_CAMERA_COMPONENT_0_TOUCHPANEL_BASE, 2);
	IOWR(NIOS_LCD_CAMERA_COMPONENT_0_TOUCHPANEL_BASE, 2, TP_val & 0x30);
}

int main() {
	alt_irq_register(NIOS_LCD_CAMERA_COMPONENT_0_TOUCHPANEL_IRQ, NULL,
			(void *) TouchPanel_int);

	printf("Experiment 3!\n");

	// initialize the touch panel
	IOWR(NIOS_LCD_CAMERA_COMPONENT_0_TOUCHPANEL_BASE, 2, 0x0);
	IOWR(NIOS_LCD_CAMERA_COMPONENT_0_TOUCHPANEL_BASE, 1, 0x400000);

	// initialize the camera
	IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CAMERA_BASE, 0, 0x0400);
	IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CAMERA_BASE, 1, 0x2);
	while ((IORD(NIOS_LCD_CAMERA_COMPONENT_0_CAMERA_BASE, 1) & 0x1) == 0)
		;
	IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CAMERA_BASE, 1, 0x4);

	// initialize the buttons
	IOWR(NIOS_LCD_CAMERA_COMPONENT_0_CONSOLE_BASE, 1, 0x0);

	// initialize the filter pipe
	IOWR(NIOS_LCD_CAMERA_COMPONENT_0_IMAGELINE_BASE, 4, 0);

	while (1);

	return 0;
}