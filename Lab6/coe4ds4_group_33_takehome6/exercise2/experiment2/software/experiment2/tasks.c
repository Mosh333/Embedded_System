// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

#include "define.h"

// For the performance counter
void *performance_name = PERFORMANCE_COUNTER_0_BASE;

// Definition of semaphore for PBs
OS_EVENT *PBSemaphore[4];

int new_priority_assign[4];
int new_born[4];

//to take care of edge case
int new_born_mult_press[4];
int new_born_and_kicked[4];

//tracking button last pressed time
long int button_times[4];
// Definition of task stacks
OS_STK initialize_task_stk[TASK_STACKSIZE];
OS_STK custom_scheduler_stk[TASK_STACKSIZE];
OS_TCB custom_scheduler_tcb;

OS_STK periodic_task_stk[NUM_TASK][TASK_STACKSIZE];
OS_TCB periodic_task_tcb[NUM_TASK];

// Struct for storing information about each custom task
typedef struct task_info_struct {
	int priority;
	int execution_time;
	int os_delay;
} task_info_struct;

// Struct for storing information about tasks during dynamic scheduling using the custom_scheduler
typedef struct scheduler_info_struct {
	int valid;
	int id;
	int period;
} scheduler_info_struct;

// Local function prototypes
void custom_delay(int);
int find_min_time(int[]);
void custom_task_handle(int, int, scheduler_info_struct[], task_info_struct[]);
void custom_scheduler(void *pdata);
//int custom_task_del(int, scheduler_info_struct[], task_info_struct[]);

scheduler_info_struct scheduler_info_holder[NUM_TASK];
// Periodic task 0
// It periodically uses a custom delay to occupy the CPU
// Then it suspends itself for a specified period of time
void periodic_task0(void* pdata) {
	task_info_struct *task_info_ptr;

	task_info_ptr = (task_info_struct *) pdata;
	while (1) {
		printf("Start periodic_task0 (%4d ms) (%1d s) (%d priority)\n",
				task_info_ptr->execution_time, task_info_ptr->os_delay,
				task_info_ptr->priority);
		custom_delay(task_info_ptr->execution_time);
		printf("End	  periodic_task0 (%4d ms) (%1d s) (%d priority)\n",
				task_info_ptr->execution_time, task_info_ptr->os_delay,
				task_info_ptr->priority);
		OSTimeDlyHMSM(0, 0, task_info_ptr->os_delay, 0);
	}
}

// Periodic task 1
// It periodically uses a custom delay to occupy the CPU
// Then it suspends itself for a specified period of time
void periodic_task1(void* pdata) {
	task_info_struct *task_info_ptr;

	task_info_ptr = (task_info_struct *) pdata;
	while (1) {
		printf("Start periodic_task1 (%4d ms) (%1d s) (%d priority)\n",
				task_info_ptr->execution_time, task_info_ptr->os_delay,
				task_info_ptr->priority);
		custom_delay(task_info_ptr->execution_time);
		printf("End	  periodic_task1 (%4d ms) (%1d s) (%d priority)\n",
				task_info_ptr->execution_time, task_info_ptr->os_delay,
				task_info_ptr->priority);
		OSTimeDlyHMSM(0, 0, task_info_ptr->os_delay, 0);
	}
}

// Periodic task 2
// It periodically uses a custom delay to occupy the CPU
// Then it suspends itself for a specified period of time
void periodic_task2(void* pdata) {
	task_info_struct *task_info_ptr;

	task_info_ptr = (task_info_struct *) pdata;
	while (1) {
		printf("Start periodic_task2 (%4d ms) (%1d s) (%d priority)\n",
				task_info_ptr->execution_time, task_info_ptr->os_delay,
				task_info_ptr->priority);
		custom_delay(task_info_ptr->execution_time);
		printf("End	  periodic_task2 (%4d ms) (%1d s) (%d priority)\n",
				task_info_ptr->execution_time, task_info_ptr->os_delay,
				task_info_ptr->priority);
		OSTimeDlyHMSM(0, 0, task_info_ptr->os_delay, 0);
	}
}

// Periodic task 3
// It periodically uses a custom delay to occupy the CPU
// Then it suspends itself for a specified period of time
void periodic_task3(void* pdata) {
	task_info_struct *task_info_ptr;

	task_info_ptr = (task_info_struct *) pdata;
	while (1) {
		printf("Start periodic_task3 (%4d ms) (%1d s) (%d priority)\n",
				task_info_ptr->execution_time, task_info_ptr->os_delay,
				task_info_ptr->priority);
		custom_delay(task_info_ptr->execution_time);
		printf("End	  periodic_task3 (%4d ms) (%1d s) (%d priority)\n",
				task_info_ptr->execution_time, task_info_ptr->os_delay,
				task_info_ptr->priority);
		OSTimeDlyHMSM(0, 0, task_info_ptr->os_delay, 0);
	}
}

int find_min_time(int PB_pressed[]) {//function to return the lowest OS time in our global array
	int min = -1;
	int i;
	int flag = 0;
	int furthest_time = 2147483647;

	for (i = 0; i < 4; i++) {
		if ((PB_pressed[i] == 1) && (button_times[i] != -1)) {//IF BUTTON TIMES [I] ==-1 THAT MEANS THE BUTTON ASSOC W/ THIS TIME HAS BEEN SERVED!! DONT DO AGAIN!!!
			if (furthest_time > button_times[i]) {
				min = i;
				furthest_time = button_times[i];
				flag = 1;

			}

		}

	}

	return min;
}

// The custom_scheduler
// It has the highest priority
// It checks the PBs every 500ms
// It a button has been pressed, it creates/deletes the corresponding task in the OS
// When creating a task, it will assign the new task with the lowest priority among the running tasks
void custom_scheduler(void *pdata) {
	INT8U return_code = OS_NO_ERR;
	int i;
	int PB_pressed[NUM_PB_BUTTON]; //if 0 not created, if 1 task created, if 2 first subsequent press, if 3 rest of subsequent press
	//int PB_flag[];
	int sem_value;
	int new_pressed;
	int lowest_priority_assign[4];//we will service or start tasks in order of this array from index 0 to 3

	int PB0_time;
	int PB1_time;
	int PB2_time;
	int PB3_time;
	int j;
	int task_num_to_serve;
	int increment_flag;

	int num_buttons_pressed;

	int num_active_task;
	// Array of task_info
	task_info_struct task_info[NUM_TASK];
	scheduler_info_struct scheduler_info[NUM_TASK];

	printf("Starting custom scheduler...\n");

	num_active_task = 0;


	// Scheduler never returns
	while (1) {
		new_pressed = 0;
		// Check for PBs
		num_buttons_pressed = 0;

		for (i = 0; i < NUM_PB_BUTTON; i++) {
			PB_pressed[i] = 0;
			sem_value = OSSemAccept(PBSemaphore[i]);
			if (sem_value != 0) {
				PB_pressed[i] = 1;
				new_pressed = 1;

				printf("Button %d was pressed at time %ld\n", i,
						button_times[i]);
				num_buttons_pressed++;

			}
		}

		//printf("\n%d buttons have been pressed\n", num_buttons_pressed);
		//printf("The value of new_pressed is %d\n", new_pressed);

		if (new_pressed != 0) {
			printf("Locking OS scheduler for new scheduling\n");
			OSSchedLock();
			for (j = 0; j < num_buttons_pressed; j++) {


				task_num_to_serve = find_min_time(PB_pressed);
				//printf("\nWe are serving task #%d\n", task_num_to_serve);

				if(new_born[task_num_to_serve]==1)increment_flag=1;
				custom_task_handle(task_num_to_serve, num_active_task,
						scheduler_info, task_info);
				button_times[task_num_to_serve] = -1;
				if(increment_flag==1){num_active_task++;increment_flag=0;}
				printf("Printing task info:\n");
				for (i = 0; i < num_active_task; i++) {
					printf(
							"Priority %d: valid=%d, task_id=%d, period=%d, exec_time=%d, os_delay=%d, pri=%d\n",
							i, scheduler_info[i].valid, scheduler_info[i].id,
							scheduler_info[i].period,
							task_info[scheduler_info[i].id].execution_time,
							task_info[scheduler_info[i].id].os_delay,
							task_info[scheduler_info[i].id].priority);
				}
			}
//			printf("Printing task info:\n");
//			for (i = 0; i < num_active_task; i++) {
//				printf(
//						"Priority %d: valid=%d, task_id=%d, period=%d, exec_time=%d, os_delay=%d, pri=%d\n",
//						i, scheduler_info[i].valid, scheduler_info[i].id,
//						scheduler_info[i].period,
//						task_info[scheduler_info[i].id].execution_time,
//						task_info[scheduler_info[i].id].os_delay,
//						task_info[scheduler_info[i].id].priority);
//			}

			printf("Unlocking OS scheduler\n");
			OSSchedUnlock();
		}

		OSTimeDly(3000);
	}
}

// Function for creating a task in the OS, and update the data structure task_info
// The new task has the lowest priority among the existing tasks
void custom_task_handle(int task_num, int num_active_task,
		scheduler_info_struct scheduler_info[], task_info_struct task_info[]) {
	//int i;
	int t;
	int k;
	int kick_this_prio;
	int num_task_created;
	INT8U return_code = OS_NO_ERR;
	int lowest_priority_assign[4];

	num_task_created = 0;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//CREATING TASKS WITH HIGHEST PRIORITY
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////





	if (new_born_and_kicked[task_num] == 1){//START NEW BORN AND KICKED excuse our wierd terms but this means that the function needs to be newly created BUT with lowest priority

		printf("Creating task(s) in the OS ...\n");
			task_info[task_num].execution_time = rand() % (1750-1000)+1000;
			task_info[task_num].os_delay =rand() % OS_DELAY_LIMIT + 1;
			task_info[task_num].priority = num_active_task;//num_active_task + num_task_created;

			scheduler_info[num_active_task].valid = 1;
			scheduler_info[num_active_task].period = rand() % (12000-10000)+10000;
			scheduler_info[num_active_task].id = task_num;

//			printf(
//					"-Creating periodic_task%d: execution_time_tick = %d, os_delay_time = %d: priority (%d)\n",
//					task_num, task_info[task_num].execution_time,
//					task_info[task_num].os_delay, 0);

			// Create the task in the OS
			printf("\n--------\nWe are creating task %d with priority %d. It should have the lowest of all current priorities\n--------\n",task_num, TASK_START_PRIORITY+num_active_task);
			switch (task_num) {
			case 0:
				return_code = OSTaskCreateExt(periodic_task0, &task_info[task_num],
						(void *) &periodic_task_stk[task_num][TASK_STACKSIZE
								- 1], TASK_START_PRIORITY+num_active_task, task_num,
						&periodic_task_stk[task_num][0], TASK_STACKSIZE,
						&periodic_task_tcb[task_num], 0);
				alt_ucosii_check_return_code(return_code);
				break;
			case 1:
				return_code = OSTaskCreateExt(periodic_task1, &task_info[task_num],
						(void *) &periodic_task_stk[task_num][TASK_STACKSIZE - 1],
						TASK_START_PRIORITY+num_active_task, task_num,
						&periodic_task_stk[task_num][0], TASK_STACKSIZE,
						&periodic_task_tcb[task_num], 0);
				alt_ucosii_check_return_code(return_code);
				break;
			case 2:
				return_code = OSTaskCreateExt(periodic_task2, &task_info[task_num],
						(void *) &periodic_task_stk[task_num][TASK_STACKSIZE - 1],
						TASK_START_PRIORITY+num_active_task, task_num,
						&periodic_task_stk[task_num][0], TASK_STACKSIZE,
						&periodic_task_tcb[task_num], 0);
				alt_ucosii_check_return_code(return_code);
				break;
			default:
				return_code = OSTaskCreateExt(periodic_task3, &task_info[task_num],
						(void *) &periodic_task_stk[task_num][TASK_STACKSIZE - 1],
						TASK_START_PRIORITY+num_active_task, task_num,
						&periodic_task_stk[task_num][0], TASK_STACKSIZE,
						&periodic_task_tcb[task_num], 0);
				alt_ucosii_check_return_code(return_code);
				break;
			}
			new_born[task_num] = 0;
			new_born_and_kicked[task_num] = 0;

	}//END NEW BORN AND KICKED

	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//CREATING TASKS WITH LOWEST PRIORITY
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	else if (new_born[task_num] == 1) {//IF NEW CREATED TASK the function needs to be newly created but with HIGHEST priority

		for (k = 0; k < NUM_TASK; k++) {
			scheduler_info_holder[k] = scheduler_info[k];
		}
		//////SHIFT ALL OTHER PRIORITIES DOWN
		//printf("\n\nENTER SHIFT algorithm:\n-------------------------------\n");

		for (t = 0; t < num_active_task; t++) {
////			printf("shifting active task: %d out of %d\n", t, num_active_task
//					- 1);
			//TASK_START_PRIORITY =10
			//TEMP_SHIFT_OFFSET=10
			//printf("\NThe t value is: %d and the num of active tasks is %d\n", t, num_active_task);
//			printf(
//					"shifting the priority of Active task( %d ) from ( %d ) to ( %d )\n",
//					t, TASK_START_PRIORITY + t, TASK_START_PRIORITY
//							+ TEMP_SHIFT_OFFSET + t + 1);

			return_code = OSTaskChangePrio(TASK_START_PRIORITY + t,
					TASK_START_PRIORITY + TEMP_SHIFT_OFFSET + t + 1);
			alt_ucosii_check_return_code(return_code);

//			printf(
//					"\nwe are now shifting scheduler_info data\n----------------------\n");
			//printf("saving scheduler")
			scheduler_info[t + 1].valid = scheduler_info_holder[t].valid;
			scheduler_info[t + 1].id = scheduler_info_holder[t].id;
			scheduler_info[t + 1].period = scheduler_info_holder[t].period;
			task_info[scheduler_info[t + 1].id].priority = t + 1;
//			printf(
//					"\ndone shifting scheduler_info data\n----------------------\n");
		}
		//printf("\n\EXIT SHIFT algorithm:\n-------------------------------\n");

		//printf("Creating task(s) in the OS ...\n");
		task_info[task_num].execution_time = rand() % (1750-1000)+1000;
		task_info[task_num].os_delay = rand() % OS_DELAY_LIMIT + 1;//rand() % (12000-10000)+10000;
		task_info[task_num].priority = 0;//num_active_task + num_task_created;

		scheduler_info[0].valid = 1;
		scheduler_info[0].period = rand() % (12000-10000)+10000;
		scheduler_info[0].id = task_num;

//		printf(
//				"-Creating periodic_task%d: execution_time_tick = %d, os_delay_time = %d: priority (%d)\n",
//				task_num, task_info[task_num].execution_time,
//				task_info[task_num].os_delay, 0);

		// Create the task in the OS
		printf("\n--------\nWe are creating task %d with priority %d. It should have the HIGHEST of all current priorities\n--------\n",task_num, TASK_START_PRIORITY);

		switch (task_num) {
		case 0:
			return_code = OSTaskCreateExt(periodic_task0, &task_info[task_num],
					(void *) &periodic_task_stk[task_num][TASK_STACKSIZE
							- 1], TASK_START_PRIORITY, task_num,
					&periodic_task_stk[task_num][0], TASK_STACKSIZE,
					&periodic_task_tcb[task_num], 0);
			alt_ucosii_check_return_code(return_code);
			break;
		case 1:
			return_code = OSTaskCreateExt(periodic_task1, &task_info[task_num],
					(void *) &periodic_task_stk[task_num][TASK_STACKSIZE - 1],
					TASK_START_PRIORITY, task_num,
					&periodic_task_stk[task_num][0], TASK_STACKSIZE,
					&periodic_task_tcb[task_num], 0);
			alt_ucosii_check_return_code(return_code);
			break;
		case 2:
			return_code = OSTaskCreateExt(periodic_task2, &task_info[task_num],
					(void *) &periodic_task_stk[task_num][TASK_STACKSIZE - 1],
					TASK_START_PRIORITY, task_num,
					&periodic_task_stk[task_num][0], TASK_STACKSIZE,
					&periodic_task_tcb[task_num], 0);
			alt_ucosii_check_return_code(return_code);
			break;
		default:
			return_code = OSTaskCreateExt(periodic_task3, &task_info[task_num],
					(void *) &periodic_task_stk[task_num][TASK_STACKSIZE - 1],
					TASK_START_PRIORITY, task_num,
					&periodic_task_stk[task_num][0], TASK_STACKSIZE,
					&periodic_task_tcb[task_num], 0);
			alt_ucosii_check_return_code(return_code);
			break;
		}

		for (t = num_active_task - 1; t >= 0; t--) {
//			printf("\nshifting active task: %d out of %d\n", t, num_active_task);
//			//TASK_START_PRIORITY =10
//			//TEMP_SHIFT_OFFSET=10
//			//printf("\NThe t value is: %d and the num of active tasks is %d\n", t, num_active_task);
//			printf(
//					"\nshifting the priority of Active task( %d ) back from ( %d ) to ( %d )\n",
//					t, TASK_START_PRIORITY + TEMP_SHIFT_OFFSET + t + 1,
//					TASK_START_PRIORITY + t + 1);
			return_code = OSTaskChangePrio(TASK_START_PRIORITY
					+ TEMP_SHIFT_OFFSET + t + 1, TASK_START_PRIORITY + t + 1);
			alt_ucosii_check_return_code(return_code);
		}

		new_born[task_num] = 0;
		num_task_created++;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//KICKING TASKS TO LOWEST PRIORITY
	///////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////




	else {//task is already created, it must be shifted to lowest priority
//		printf(
//				"Task #%d is not a newborn aka this is a SUBSEQUENT PRESS!!\n\n",
//				task_num);

		kick_this_prio = task_info[task_num].priority + 10;

		if(kick_this_prio-10==num_active_task-1){
			printf("This task is ALREADY the lowest priority, dont do anything\n");
			return;//do nothing
		}else{


		//printf("We are kicking the task with priority %d to the bottom\n", kick_this_prio);
			printf("\n--------\nWe are kicking task %d with priority %d. It should have the lowest of all current priorities aka %d\n--------\n",task_num, kick_this_prio,  num_active_task+10-1);


		//need to find the priority of the task that we need to shift lowest

		//	if multiple buttons were pressed between 2 runs of the scheduler, we need to find most recent presses and assign those highest priority

		for (k = 0; k < NUM_TASK; k++) {
			scheduler_info_holder[k] = scheduler_info[k];
		}


		//shift out the prio that needs to be kicked

//		return_code = OSTaskChangePrio(kick_this_prio ,15);
//		alt_ucosii_check_return_code(return_code);

		//printf("\nwe are now shifting scheduler_info data for prio to kick\n----------------------\n");
		//printf("saving scheduler")
		scheduler_info[num_active_task-1].valid = scheduler_info_holder[kick_this_prio-10].valid;
		scheduler_info[num_active_task-1].id = scheduler_info_holder[kick_this_prio-10].id;
		scheduler_info[num_active_task-1].period = scheduler_info_holder[kick_this_prio-10].period;
		task_info[scheduler_info[num_active_task-1].id].priority = num_active_task-1;
		//printf("\ndone shifting scheduler_info data for prio to kick\n----------------------\n");
		//////SHIFT ALL OTHER PRIORITIES DOWN
		//printf("\nENTER SHIFT algorithm:\n-------------------------------\n");


				for (t = kick_this_prio-10; t < num_active_task; t++) {

						//printf("shifting active task: %d out of %d\n", t,num_active_task - 1);
						//printf("shifting the priority of Active task( %d ) from ( %d ) to ( %d )\n",t, TASK_START_PRIORITY + t, TASK_START_PRIORITY+TEMP_SHIFT_OFFSET+t);

						return_code = OSTaskChangePrio(TASK_START_PRIORITY + t,TASK_START_PRIORITY+TEMP_SHIFT_OFFSET+t);
						alt_ucosii_check_return_code(return_code);

										//printf("\nwe are now shifting scheduler_info data\n----------------------\n");
										//printf("saving scheduler")
						if(t != kick_this_prio-10){
										scheduler_info[t - 1].valid = scheduler_info_holder[t].valid;
										scheduler_info[t - 1].id = scheduler_info_holder[t].id;
										scheduler_info[t - 1].period = scheduler_info_holder[t].period;
										task_info[scheduler_info[t - 1].id].priority = t-1 ;
										//printf("\ndone shifting scheduler_info data\n----------------------\n");
										//printf("\n\EXIT SHIFT algorithm:\n-------------------------------\n");
						}
				}
				//printf("shifting the priority of kicked Active task( %d ) back from ( %d ) to ( %d )\n",kick_this_prio,kick_this_prio+TEMP_SHIFT_OFFSET,num_active_task-1);
				return_code = OSTaskChangePrio(kick_this_prio+TEMP_SHIFT_OFFSET,TASK_START_PRIORITY+num_active_task-1);
				alt_ucosii_check_return_code(return_code);
				for (t = kick_this_prio-10+1; t < num_active_task; t++) {

						//printf("shifting active task: %d out of %d\n", t,num_active_task - 1);
						//printf("shifting the priority of Active task( %d ) back from ( %d ) to ( %d )\n",t, TASK_START_PRIORITY+TEMP_SHIFT_OFFSET+t,TASK_START_PRIORITY + t-1);

						return_code = OSTaskChangePrio(TASK_START_PRIORITY+TEMP_SHIFT_OFFSET+t,TASK_START_PRIORITY + t-1);
						alt_ucosii_check_return_code(return_code);
				}




//		for (t = kick_this_prio+1-10; t < num_active_task; t++) {
//
//				printf("shifting active task: %d out of %d\n", t,
//						num_active_task - 1);
//				printf(
//						"shifting the priority of Active task( %d ) from ( %d ) to ( %d )\n",
//						t, TASK_START_PRIORITY + t, kick_this_prio+t -1);
//
//				return_code = OSTaskChangePrio(TASK_START_PRIORITY + t,
//						kick_this_prio+t -1);
//				alt_ucosii_check_return_code(return_code);
//
//				//printf("\nwe are now shifting scheduler_info data\n----------------------\n");
//				//printf("saving scheduler")
//				scheduler_info[t - 1].valid = scheduler_info_holder[t].valid;
//				scheduler_info[t - 1].id = scheduler_info_holder[t].id;
//				scheduler_info[t - 1].period = scheduler_info_holder[t].period;
//				task_info[scheduler_info[t - 1].id].priority = t - 1;
//				//printf("\ndone shifting scheduler_info data\n----------------------\n");
//				//printf("\n\EXIT SHIFT algorithm:\n-------------------------------\n");
//
//
//		}
		//printf("\nEXITING SHIFT algorithm:\n-------------------------------\n");

		//printf("shifting priority task %d from 15 back down to %d\n", kick_this_prio,num_active_task);
//		return_code = OSTaskChangePrio(15 ,num_active_task+10-1);
//		alt_ucosii_check_return_code(return_code);


	}
	}

}

// Function for occupying the processor for the specified number of clock ticks
// to simulate custom delay while keeping the task in the processor
void custom_delay(int ticks) {
	INT32U cur_tick;
	ticks--;
	cur_tick = OSTimeGet();
	while (ticks > 0) {
		if (OSTimeGet() > cur_tick) {
			ticks--;
			cur_tick = OSTimeGet();
		}
	}
}
