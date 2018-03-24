// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Digital Systems Design course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

#ifndef __PS2_CONTROLLER_H__
#define __PS2_CONTROLLER_H__

// Global functions
void handle_PS2_interrupts(PS2_buffer_struct *);
void read_PS2_data(int *, int *);
void init_PS2_irq(PS2_buffer_struct *);

#endif
