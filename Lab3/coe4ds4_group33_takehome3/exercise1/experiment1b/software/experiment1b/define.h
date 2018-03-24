// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

#ifndef	  __define_H__
#define	  __define_H__

#include <io.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "sys/alt_alarm.h"
#include "alt_types.h"
#include "system.h"
#include <sys/alt_irq.h>
#include "PB_button.h"
#include "custom_counter_0.h"
#include "custom_counter_1.h"
#include "elevator_sim.h"
#include "switches.h"




volatile typedef struct elevator{
	int motion;//1 for up, 0 for still, -1 for down
	int current_floor;
	int next_floor;
	int direction;
	int request_queue[12];
	int door_close_ready;

	int keep_door_open;

	int door_open_time;
	int floor_travel_time;
	int counter_1_running;
	int arrived_at_floor;


}elevator_struct;






#endif
