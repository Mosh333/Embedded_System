// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

// This shows register details for the 
// NIOS_LCD_COMPONENT / NIOS_LCD_CAMERA_COMPONENT

Both peripherals (with and without the Camera) contain the 
Touchpanel slave and the Console slave. The image slave is used 
in experiment 2 and interfaces the SDRAM to Nios for line by line
writes. Ther camera slave is provides configuration access to the 
camera as well as handling capture start/stop and appears in both
experiments 2 and 3. The Imageline slave also appears in experiments 
2 and 3, but is used differently in the two cases. 

NIOS_LCD_COMPONENT for experiment 1

	The Touchpanel slave has three registers, offset 0 read 
	gives the touch point coordinates, the interval (in terms
	of 50 MHz clock cycles) for the periodic interrupt is 
	written to offset 1 and status is read/written in offset 2.

	TOUCHPANEL
		TOUCHPANEL_BASE + 0 
			bit  31    - Touch_En
			bits 27:16 - x_coord
			bits 11:0  - y_coord
		TOUCHPANEL_BASE + 1 
			bits 31:0 - Interrupt intverval
		TOUCHPANEL_BASE + 2 
			bit 5 - Touch_En
			bit 4 - posedge Coord_En
			bit 3 - posedge Timer
			bit 2 - negedge Touch_En
			bit 1 - posedge Touch_En
			bit 0 - IRQ

	The console has a very simple interface, write bits in
	offset 0 to lite up/turn off the corresponding key
	
	CONSOLE
		CONSOLE_BASE + 0
			bits 5:0 - invert key 5:0
	
	The Image slave allows writes to the SDRAM in stream
	mode (i.e. pixel by pixel along a line, one line at a
	time). Reading offset 1 will give the values of the 
	hardware row/column counters, and a write to offset 2 
	will resynchronize the write point to the top left-hand 
	corner of the display area (pixel 0,0)
 
	IMAGE
		IMAGE_BASE + 0
			bits 31:0 - RGB write to SDRAM stream
		IMAGE_BASE + 1
			bits 24:16 - Current Y position
			bits 9:0 - Current X position
		IMAGE_BASE + 2
			Synchronize to top-left corner

	The Camera slave is for configuration, offset 0 contains
	the exposure time for setting up brightness/frame rate. 
	Offset 1 provides capture and config control and reading
	offset 2 gives the value of the internal hardware 
	frame counter. 
	
NIOS_LCD_CAMERA_COMPONENT for experiments 2 and 3

	CAMERA
		CAMERA_BASE + 0
			bits 15:0 - exposure
		CAMERA_BASE + 1
			bit 3 - stop capture
			bit 2 - start capture
			bit 1 - start config
			bit 0 - config done
		CAMERA_BASE + 2
			bits 31:0 - frame counter

	The Imageline slave is used to provide 	read/write access 
	to the SDRAM in stream mode as well as configuration data
	to the custom hardware filter. 	Offset 0 reads/writes step 
	through the image pixel by pixel returning/using 32-bit 
	RGB values. Offset 1 sets up the source for read/write 
	transactions on the SDRAM, 1 indicates Nios, 0 indicates 
	the hardware (Camera/LCD), bit 1 sets up the write port, 
	bit 0 the read port. Bit 4 triggers a state reload, with 
	bits 3 and 2 indicating whether to provide reset to the 
	write and read ports of the SDRAM respectively. Finally, 
	offset 4 is the 32 bit configuration word for the custom 
	filter in hardware.

	IMAGELINE
		IMAGELINE_BASE + 0
			bits 31:0 - RGB read/write from/to SDRAM stream
		IMAGELINE_BASE + 1
			bit 4 - trigger state reload
			bit 3 - write port reset on state reload
			bit 2 - read port reset on state reload
			bit 1 - write source
			bit 0 - read source
		IMAGELINE + 4
			bits 31:0 - Filter configuration word
			
