// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

#ifndef __task_H__
#define __task_H__

// Size of stack for each task
#define	  TASK_STACKSIZE	   2048


// Definition of Task Priorities////////////////////////////
#define INITIALIZE_TASK_PRIORITY   5

//Define Mutex Priority
#define SW_GRPA_MUTEX_PRIORITY		   6
#define SW_GRPB_MUTEX_PRIORITY		   7
#define RED_LED_MUTEX_PRIORITY		   8
#define GREEN_LED_MUTEX_PRIORITY	   9

#define TASK_START_PRIORITY		  10
#define TASK_LAUNCHER_PRIORITY	  18

//custom task priority
#define CUSTOM_TASK_3_PRIORITY	  11
#define CUSTOM_TASK_2_PRIORITY	  12
#define CUSTOM_TASK_1_PRIORITY	  13
#define CUSTOM_TASK_0_PRIORITY	  14


////////////////////////////////////////////////////////////




// Number of custom tasks
#define NUM_TASK 4


// Number of clock ticks for delaying the LCD display
#define DELAY_TICK 1000

//Task Duration
#define time0 1650 //group 33
#define time1 1450 //group 33
#define time2 1450 //Thursday
#define time3 1250 //Thursday

// Global function
void task_launcher(void *pdata);

#endif
