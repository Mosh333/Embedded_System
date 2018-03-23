// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

/* 
 * "Small Hello World" example. 
 * 
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example 
 * designs. It requires a STDOUT  device in your system's hardware. 
 *
 * The purpose of this example is to demonstrate the smallest possible Hello 
 * World application, using the Nios II HAL library.  The memory footprint
 * of this hosted application is ~332 bytes by default using the standard 
 * reference design.  For a more fully featured Hello World application
 * example, see the example titled "Hello World".
 *
 * The memory footprint of this example has been reduced by making the
 * following changes to the normal "Hello World" example.
 * Check in the Nios II Software Developers Manual for a more complete 
 * description.
 * 
 * In the SW Application project (small_hello_world):
 *
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 * In System Library project (small_hello_world_syslib):
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 *    - Define the preprocessor option ALT_NO_INSTRUCTION_EMULATION 
 *      This removes software exception handling, which means that you cannot 
 *      run code compiled for Nios II cpu with a hardware multiplier on a core 
 *      without a the multiply unit. Check the Nios II Software Developers 
 *      Manual for more details.
 *
 *  - In the System Library page:
 *    - Set Periodic system timer and Timestamp timer to none
 *      This prevents the automatic inclusion of the timer driver.
 *
 *    - Set Max file descriptors to 4
 *      This reduces the size of the file handle pool.
 *
 *    - Check Main function does not exit
 *    - Uncheck Clean exit (flush buffers)
 *      This removes the unneeded call to exit when main returns, since it
 *      won't.
 *
 *    - Check Don't use C++
 *      This builds without the C++ support code.
 *
 *    - Check Small C library
 *      This uses a reduced functionality C library, which lacks  
 *      support for buffering, file IO, floating point and getch(), etc. 
 *      Check the Nios II Software Developers Manual for a complete list.
 *
 *    - Check Reduced device drivers
 *      This uses reduced functionality drivers if they're available. For the
 *      standard design this means you get polled UART and JTAG UART drivers,
 *      no support for the LCD driver and you lose the ability to program 
 *      CFI compliant flash devices.
 *
 *    - Check Access device drivers directly
 *      This bypasses the device file system to access device drivers directly.
 *      This eliminates the space required for the device file system services.
 *      It also provides a HAL version of libc services that access the drivers
 *      directly, further reducing space. Only a limited number of libc
 *      functions are available in this configuration.
 *
 *    - Use ALT versions of stdio routines:
 *
 *           Function                  Description
 *        ===============  =====================================
 *        alt_printf       Only supports %s, %x, and %c ( < 1 Kbyte)
 *        alt_putstr       Smaller overhead than puts with direct drivers
 *                         Note this function doesn't add a newline.
 *        alt_putchar      Smaller overhead than putchar with direct drivers
 *        alt_getchar      Smaller overhead than getchar with direct drivers
 *
 */

#include "system.h"
#include <io.h>
#include "sys/alt_stdio.h"
#include "altera_up_avalon_character_lcd.h"
#include "sys/alt_irq.h"

int main() {
	alt_u16 a = 0x1;
	alt_u32 b = 0;
	alt_u32 i;
	alt_u8 start = 0;
	alt_16 switch_val;
	alt_16 switch_val_buf;

	alt_u32 switch_val_full_bits;
	//alt_u16 switch_val_buf;

	alt_u8 sw_16_val = 0;
	alt_u8 sw_16_val_buf = 0;
	alt_32 largest = 0;
	alt_u32 largest_index = 0;
	alt_32 second_largest = 0;

	alt_u32 longestlength = 1;
	alt_u32 begin = 0;
	alt_u32 end = 0;
	alt_u32 length = 1;

	alt_u32 largest_offset = 0;
	alt_u32 second_largest_offset = 0;
	char printStr_0[12]; //largest
	char printStr_1[12]; //2nd largest

	//	char printStr_3[12][16];//16 register values
	//	char printStr_4[12][16];//longest sequence values


	alt_16 switch_val_array[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0 };

	alt_up_character_lcd_dev *lcd_0;

	alt_printf("Experiment 3a:\n");

	lcd_0 = alt_up_character_lcd_open_dev(CHARACTER_LCD_0_NAME);


	if (lcd_0 == NULL)
		alt_printf("Error opening LCD device\n");
	else
		alt_printf("LCD device opened.\n");


	/* Event loop never exits. */
	while (1) {
		//16 bit and 32 bit versions for 16 and 17 bit lose
		switch_val_full_bits = IORD(SWITCH_I_BASE, 0);
		switch_val = IORD(SWITCH_I_BASE, 0);

		IOWR(LED_RED_O_BASE, 0, switch_val_full_bits);

		if (start == 0) {
			switch_val_buf = switch_val;
		}
		start = 1;

		//////////////

		if (switch_val_buf != switch_val) { //DETECT CHANGE IN SWITCH CONFIGURATION AND PRINT SEQUENCE OF 16 PREV INPUTS

			//shift up values if we detect shift in switches config
			for (i = 15; i > 0; i--) {
				switch_val_array[i] = switch_val_array[i - 1];
			}
			switch_val_array[0] = switch_val;

			//			alt_printf("\n Array Values are: ");
			//
			//			for (i = 0; i < 15; i++) {
			//				//printf("Printing \n ");
			//
			//				printf("%d ,", switch_val_array[i]);
			//				//printf("\n");
			//			}
			//			alt_up_character_lcd_init(lcd_0);
			//			alt_up_character_lcd_set_cursor_pos(lcd_0, 0, 1);
			//			alt_up_character_lcd_string(lcd_0, "Switches Changed!!!");
		} else {
			//			alt_up_character_lcd_init(lcd_0);
			//
			//			alt_up_character_lcd_string(lcd_0, "Experiment 3a");
			//
			//			alt_up_character_lcd_set_cursor_pos(lcd_0, 0, 1);
			//
			//			alt_up_character_lcd_string(lcd_0, "Welcome");

		}

		sw_16_val = (switch_val_full_bits >> 16) & (0x1);

		//sort largest and 2nd largest values

		largest = -1000000;
		largest_index = 0;
		second_largest = -10000000;

		for (i = 0; i < 16; i++) {
			if (switch_val_array[i] > largest) {
				largest = switch_val_array[i];
				largest_index = i;
				if (i == 15) {

				}
			}
		}

		for (i = 0; i < 16; i++) {
			if (i != largest_index) {

				if (switch_val_array[i] > second_largest) {
					second_largest = switch_val_array[i];
				}
			}

		}
		//////////////////////////////////////////////
		//		//finding longest decreasing sublist
		//		length=1;
		//		longestlength=1;
		//		begin=0;
		//		end=0;
		length = 1;
		longestlength = 1;
		begin = 0;
		end = 0;
		for (i = 0; i < 15; i++) {
			if (switch_val_array[i] > switch_val_array[i + 1]) {
				length++;
				if (length > longestlength) { //subSeqLength > longestCurrentLength
					longestlength = length;
					begin = i - (length - 2);
					end = i + 1;
				}
			} else {
				length = 1;
			}
		}

		//		char printStr_0[12]; //largest
		//		char printStr_1[12]; //2nd largest
		//
		//		char printStr_3[16][12];//16 register values
		//		char printStr_4[16][12];//longest sequence values

		sprintf(printStr_0, "%d", largest);
		sprintf(printStr_1, "%d", second_largest);

		//cast to str  16 PAST VALUES
		//		for(i=0;i<16;i++){
		//					sprintf(printStr_3[i], "%d",switch_val_array[i]);
		//				}
		//		//cast to str  LONGEST SEQUENCE
		//		for(i=begin;i<=end;i++){
		//			sprintf(printStr_4[i], "%d",switch_val_array[i]);
		//		}


		//		char printStr_0[12]; //largest
		//		char printStr_1[12]; //2nd largest
		//
		//		char printStr_3[16][12];//16 register values
		//		char printStr_4[16][12];//longest sequence values

		if (largest >= 10000)
			largest_offset = 12;
		if (largest < 10000)
			largest_offset = 13;
		if (largest < 1000)
			largest_offset = 14;
		if (largest < 100)
			largest_offset = 15;
		if (largest < 10)
			largest_offset = 16;
		if (largest < 0)
			largest_offset = 15;
		if (largest < (-10))
			largest_offset = 14;
		if (largest < (-100))
			largest_offset = 13;
		if (largest < (-1000))
			largest_offset = 12;
		if (largest < (-10000))
			largest_offset = 11;

		if (second_largest >= 10000)
			second_largest_offset = 12;
		if (second_largest < 10000)
			second_largest_offset = 13;
		if (second_largest < 1000)
			second_largest_offset = 14;
		if (second_largest < 100)
			second_largest_offset = 15;
		if (second_largest < 10)
			second_largest_offset = 16;
		if (second_largest < 0)
			second_largest_offset = 15;
		if (second_largest < (-10))
			second_largest_offset = 14;
		if (second_largest < (-100))
			second_largest_offset = 13;
		if (second_largest < (-1000))
			second_largest_offset = 12;
		if (second_largest < (-10000))
			second_largest_offset = 11;

		//printf("The largest offset is %d, ", largest_offset);


		if (!sw_16_val_buf && sw_16_val) {

			alt_up_character_lcd_init(lcd_0);
			alt_up_character_lcd_set_cursor_pos(lcd_0, largest_offset - 1, 0);

			alt_up_character_lcd_string(lcd_0, printStr_0);

			alt_up_character_lcd_set_cursor_pos(lcd_0, second_largest_offset
					- 1, 1);

			//alt_up_character_lcd_string(lcd_0,second_largest);
			alt_up_character_lcd_string(lcd_0, printStr_1);

		}

		//falling edge sw 16
		if (sw_16_val_buf && !sw_16_val) {

			//	printf("\n Switch 16 Low!! Start is: %d and end is: %d Longest Sequence length is: %d Longest Sequence values: ", begin, end, longestlength);


			//for (i = 0; i < 15; i++) {
			//		printf("%d ", switch_val_array[i]);
			//printf("\n");
			//}
			printf("\n----------------------------------------------------\n");
			printf("The ENTIRE 16-value recorded sequence is: \n");

			for (i = 0; i < 16; i++) {
				//printf("Printing \n ");

				printf("%d, ", switch_val_array[i]);
				//printf("\n");
			}
			if (begin == 0 && end == 0) {
				printf("\nNo sub-sequence exists :( \n");
			} else {
			printf("\n \nThe LONGEST sequence is: \n");

			for(i=begin;i<=end;i++) {
				printf("%d, ", switch_val_array[i]);
			}
			printf("\n----------------------------------------------------\n");

		}
	}
	usleep(100000);// SLEEP BECAUSE DEWBOUNCING PLS DONT SUE ME
	sw_16_val_buf = sw_16_val;
	switch_val_buf = switch_val;

}
}

//return 0;
