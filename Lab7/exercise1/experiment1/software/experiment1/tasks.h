// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

#ifndef __task_H__
#define __task_H__

// Size of stack for each task
#define	  TASK_STACKSIZE	   2048

// Definition of Task Priorities
#define INITIALIZE_TASK_PRIORITY   4
#define SORTING_PRIORITY		  6


#define GEN_ARRAY_0_PRIORITY	  7
#define GEN_ARRAY_1_PRIORITY	  8
#define GEN_ARRAY_2_PRIORITY	  9
#define GEN_ARRAY_3_PRIORITY	  10

#define BUBBLE_SORT_0_PRIORITY		 11
#define BUBBLE_SORT_1_PRIORITY		  12
#define BUBBLE_SORT_2_PRIORITY		  13
#define BUBBLE_SORT_3_PRIORITY		  14

#define MERGE_SORT_PRIORITY  			15

#define TASK_LAUNCHER_PRIORITY	  16

// Global function
void task_launcher(void *pdata);

#endif
