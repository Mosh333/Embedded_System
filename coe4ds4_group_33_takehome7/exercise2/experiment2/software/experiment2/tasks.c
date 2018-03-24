// Copyright by Adam Kinsman, Henry Ko and Nicola Nicolici
// Developed for the Embedded Systems course (COE4DS4)
// Department of Electrical and Computer Engineering
// McMaster University
// Ontario, Canada

#include "define.h"

// For the performance counter
void *performance_name = (void *)PERFORMANCE_COUNTER_0_BASE;

#define SD_PRESENCE_FLAG 0x1
#define SD_FILESYSTEM_FLAG 0x2
#define SD_READ_NEED_FILL 0x4
#define SD_READ_DATA_READY 0x8
#define SD_WRITE_NEED_FILL 0x10
#define SD_WRITE_DATA_READY 0x20
#define SD_PROCESS_Y_READY 0x40

// Definition of semaphore for PBs
OS_EVENT *PBSemaphore[4];

// Definition of flag
OS_FLAG_GRP *SDCardFlag;

// Definition of mutex
OS_EVENT *SDMutex;

// Definition of mailbox
//no need
OS_EVENT *YMailbox;
OS_EVENT *RMailbox;
OS_EVENT *GMailbox;
OS_EVENT *BMailbox;
OS_EVENT *YImageWidthMailbox;
OS_EVENT *YImageHeightMailbox;
OS_EVENT *ReadImageWidthMailbox;
OS_EVENT *ReadImageHeightMailbox;
OS_EVENT *WriteImageWidthMailbox;
OS_EVENT *WriteImageHeightMailbox;

// Definition of message queue
OS_EVENT *SDWriteQueueR;
OS_EVENT *SDWriteQueueG;
OS_EVENT *SDWriteQueueB;
OS_EVENT *SDReadQueue;

// memory partition
OS_MEM	*MemoryPartition;

// Definition of task stacks
OS_STK	  initialize_task_stk[TASK_STACKSIZE];
OS_STK	  task_launcher_stk[TASK_STACKSIZE];

OS_STK	  SD_presence_detect_stk[TASK_STACKSIZE];
OS_TCB	  SD_presence_detect_tcb;
OS_STK	  SD_write_stk[TASK_STACKSIZE];
OS_TCB	  SD_write_tcb;
OS_STK	  SD_read_stk[TASK_STACKSIZE];
OS_TCB	  SD_read_tcb;
OS_STK	  compute_Y_stk[TASK_STACKSIZE];
OS_TCB	  compute_Y_tcb;
OS_STK	  process_Y_stk[TASK_STACKSIZE];
OS_TCB	  process_Y_tcb;

typedef struct bmp_header_struct {
	unsigned short int magic_number;
	unsigned int file_size;
	unsigned int data_offset;
	unsigned int header_size;
	int width;
	int height;
	short int num_plane;
	short int num_bits_per_pixel;
	int compress_mode;
	int data_size;
	int hor_resolution;
	int ver_resolution;
	int num_color_in_palette;
	int important_color;
} bmp_header_struct;

void SD_presence_detect_task(void* pdata) {
	INT8U return_code = OS_NO_ERR;
	OS_FLAGS SD_flag_value;
	char filename[13];
	int status;
	printf("We are in SD_presence_detect_task()!\n");
	while (1) {
		SD_flag_value = OSFlagQuery(SDCardFlag, &return_code);
		if ((SD_flag_value & SD_PRESENCE_FLAG) == 0) {
			if (sd_card_is_Present()) {
				printf("SD card is inserted\n");
				OSFlagPost(SDCardFlag, SD_PRESENCE_FLAG, OS_FLAG_SET, &return_code);
				alt_ucosii_check_return_code(return_code);
			
				if (sd_card_is_FAT16()) {
					printf("Valid filesystem is detected on SD card\n");
					OSFlagPost(SDCardFlag, SD_FILESYSTEM_FLAG, OS_FLAG_SET, &return_code);
					alt_ucosii_check_return_code(return_code);	
					
					status = sd_card_find_first(".", filename);
					switch (status) {
	                    case 0: printf("Success\n"); break;
	                    case 1: printf("Invalid directory\n"); break;
	                    case 2: printf("No card or incorrect FS\n"); break;
	                }
				}
			}
		} else {
			if (!sd_card_is_Present()) {
				printf("SD card is removed\n");
				
				// clear all the flags
				OSFlagPost(SDCardFlag, 0xFF, OS_FLAG_CLR, &return_code);
				alt_ucosii_check_return_code(return_code);
			}
		}

		// Check if SD card is present every 500ms
		OSTimeDlyHMSM(0, 0, 0, 500);
	}
}

void SD_read_task(void *pdata) {
	INT8U return_code = OS_NO_ERR;
	short int file_handle, file_handle1, file_handle2, file_handle3;
	unsigned short int data, data1, data2, data3;
	unsigned short int finalImageData;
	int file_opened = 0;
	int image_size;
	int colCounter = 0;
	int rowCounter = 0;
	int pixelCounter = 0;
	bmp_header_struct bmp_header, bmp_header1, bmp_header2, bmp_header3;
	printf("We are in SD_read_task()!\n");
	while (1) {
		OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG | SD_READ_NEED_FILL, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
		alt_ucosii_check_return_code(return_code);
		
		// printf("R: waiting for SDMutex\n");
        // OSTimeDlyHMSM(0, 0, 0, 100);
		OSMutexPend(SDMutex, 0, &return_code);
		alt_ucosii_check_return_code(return_code);
		
		if (file_opened == 0) {
			//The order here matters since we are reading positive height images, we we bottom ones first, then the top ones second
				file_handle = sd_card_fopen("INPIC2.BMP", false);
				file_handle1 = sd_card_fopen("INPIC3.BMP", false);
				file_handle2 = sd_card_fopen("INPIC0.BMP", false);
				file_handle3 = sd_card_fopen("INPIC1.BMP", false);

				if (file_handle < 0) {
					printf("Error opening INPIC0.BMP\n");
					OSTimeDlyHMSM(0, 0, 1, 0);
					break;
				}
				if (file_handle1 < 0) {
					printf("Error opening INPIC1.BMP\n");
					OSTimeDlyHMSM(0, 0, 1, 0);
					break;
				}
				if (file_handle2 < 0) {
					printf("Error opening INPIC2.BMP\n");
					OSTimeDlyHMSM(0, 0, 1, 0);
					break;
				}
				if (file_handle3 < 0) {
					printf("Error opening INPIC3.BMP\n");
					OSTimeDlyHMSM(0, 0, 1, 0);
					break;
				}
			printf("Image opened: INPIC0.BMP\n");
			// BMP header
			data = sd_card_read(file_handle);
			bmp_header.magic_number = data << 8;
			data = sd_card_read(file_handle);
			bmp_header.magic_number = bmp_header.magic_number | data;
			
			data = sd_card_read(file_handle);
			bmp_header.file_size = data;
			data = sd_card_read(file_handle);
			bmp_header.file_size = bmp_header.file_size | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.file_size = bmp_header.file_size | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.file_size = bmp_header.file_size | data << 24;
			
			// unused
			data = sd_card_read(file_handle);
			data = sd_card_read(file_handle);
			data = sd_card_read(file_handle);
			data = sd_card_read(file_handle);

			data = sd_card_read(file_handle);
			bmp_header.data_offset = data;
			data = sd_card_read(file_handle);
			bmp_header.data_offset = bmp_header.data_offset | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.data_offset = bmp_header.data_offset | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.data_offset = bmp_header.data_offset | data << 24;

			data = sd_card_read(file_handle);
			bmp_header.header_size = data;
			data = sd_card_read(file_handle);
			bmp_header.header_size = bmp_header.header_size | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.header_size = bmp_header.header_size | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.header_size = bmp_header.header_size | data << 24;

			data = sd_card_read(file_handle);
			bmp_header.width = data;
			data = sd_card_read(file_handle);
			bmp_header.width = bmp_header.width | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.width = bmp_header.width | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.width = bmp_header.width | data << 24;

			data = sd_card_read(file_handle);
			bmp_header.height = data;
			data = sd_card_read(file_handle);
			bmp_header.height = bmp_header.height | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.height = bmp_header.height | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.height = bmp_header.height | data << 24;

			data = sd_card_read(file_handle);
			bmp_header.num_plane = data;
			data = sd_card_read(file_handle);
			bmp_header.num_plane = bmp_header.num_plane | data << 8;

			data = sd_card_read(file_handle);
			bmp_header.num_bits_per_pixel = data;
			data = sd_card_read(file_handle);
			bmp_header.num_bits_per_pixel = bmp_header.num_bits_per_pixel | data << 8;
			
			data = sd_card_read(file_handle);
			bmp_header.compress_mode = data;
			data = sd_card_read(file_handle);
			bmp_header.compress_mode = bmp_header.compress_mode | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.compress_mode = bmp_header.compress_mode | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.compress_mode = bmp_header.compress_mode | data << 24;

			data = sd_card_read(file_handle);
			bmp_header.data_size = data;
			data = sd_card_read(file_handle);
			bmp_header.data_size = bmp_header.data_size | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.data_size = bmp_header.data_size | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.data_size = bmp_header.data_size | data << 24;
								
			data = sd_card_read(file_handle);
			bmp_header.hor_resolution = data;
			data = sd_card_read(file_handle);
			bmp_header.hor_resolution = bmp_header.hor_resolution | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.hor_resolution = bmp_header.hor_resolution | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.hor_resolution = bmp_header.hor_resolution | data << 24;
			
			data = sd_card_read(file_handle);
			bmp_header.ver_resolution = data;
			data = sd_card_read(file_handle);
			bmp_header.ver_resolution = bmp_header.ver_resolution | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.ver_resolution = bmp_header.ver_resolution | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.ver_resolution = bmp_header.ver_resolution | data << 24;
			
			data = sd_card_read(file_handle);
			bmp_header.num_color_in_palette = data;
			data = sd_card_read(file_handle);
			bmp_header.num_color_in_palette = bmp_header.num_color_in_palette | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.num_color_in_palette = bmp_header.num_color_in_palette | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.num_color_in_palette = bmp_header.num_color_in_palette | data << 24;
			
			data = sd_card_read(file_handle);
			bmp_header.important_color = data;
			data = sd_card_read(file_handle);
			bmp_header.important_color = bmp_header.important_color | data << 8;
			data = sd_card_read(file_handle);
			bmp_header.important_color = bmp_header.important_color | data << 16;
			data = sd_card_read(file_handle);
			bmp_header.important_color = bmp_header.important_color | data << 24;
			
			printf("Reading image of dimension: %d x %d\n", bmp_header.width, bmp_header.height);
			if (bmp_header.height > 0) {
				printf("Positive height detected, image will be loaded as upside-down\n");
			}
			
			printf("Image opened: INPIC1.BMP\n");
			// BMP header 1
			data1 = sd_card_read(file_handle1);
			bmp_header1.magic_number = data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.magic_number = bmp_header1.magic_number | data1;

			data1 = sd_card_read(file_handle1);
			bmp_header1.file_size = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.file_size = bmp_header1.file_size | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.file_size = bmp_header1.file_size | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.file_size = bmp_header1.file_size | data1 << 24;

			// unused
			data1 = sd_card_read(file_handle1);
			data1 = sd_card_read(file_handle1);
			data1 = sd_card_read(file_handle1);
			data1 = sd_card_read(file_handle1);

			data1 = sd_card_read(file_handle1);
			bmp_header1.data_offset = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.data_offset = bmp_header1.data_offset | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.data_offset = bmp_header1.data_offset | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.data_offset = bmp_header1.data_offset | data1 << 24;

			data1 = sd_card_read(file_handle1);
			bmp_header1.header_size = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.header_size = bmp_header1.header_size | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.header_size = bmp_header1.header_size | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.header_size = bmp_header1.header_size | data1 << 24;

			data1 = sd_card_read(file_handle1);
			bmp_header1.width = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.width = bmp_header1.width | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.width = bmp_header1.width | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.width = bmp_header1.width | data1 << 24;

			data1 = sd_card_read(file_handle1);
			bmp_header1.height = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.height = bmp_header1.height | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.height = bmp_header1.height | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.height = bmp_header1.height | data1 << 24;


			data1 = sd_card_read(file_handle1);
			bmp_header1.num_plane = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.num_plane = bmp_header1.num_plane | data1 << 8;

			data1 = sd_card_read(file_handle1);
			bmp_header1.num_bits_per_pixel = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.num_bits_per_pixel = bmp_header1.num_bits_per_pixel | data1 << 8;

			data1 = sd_card_read(file_handle1);
			bmp_header1.compress_mode = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.compress_mode = bmp_header1.compress_mode | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.compress_mode = bmp_header1.compress_mode | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.compress_mode = bmp_header1.compress_mode | data1 << 24;

			data1 = sd_card_read(file_handle1);
			bmp_header1.data_size = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.data_size = bmp_header1.data_size | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.data_size = bmp_header1.data_size | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.data_size = bmp_header1.data_size | data1 << 24;

			data1 = sd_card_read(file_handle1);
			bmp_header1.hor_resolution = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.hor_resolution = bmp_header1.hor_resolution | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.hor_resolution = bmp_header1.hor_resolution | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.hor_resolution = bmp_header1.hor_resolution | data1 << 24;

			data1 = sd_card_read(file_handle1);
			bmp_header1.ver_resolution = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.ver_resolution = bmp_header1.ver_resolution | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.ver_resolution = bmp_header1.ver_resolution | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.ver_resolution = bmp_header1.ver_resolution | data1 << 24;

			data1 = sd_card_read(file_handle1);
			bmp_header1.num_color_in_palette = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.num_color_in_palette = bmp_header1.num_color_in_palette | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.num_color_in_palette = bmp_header1.num_color_in_palette | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.num_color_in_palette = bmp_header1.num_color_in_palette | data1 << 24;

			data1 = sd_card_read(file_handle1);
			bmp_header1.important_color = data1;
			data1 = sd_card_read(file_handle1);
			bmp_header1.important_color = bmp_header1.important_color | data1 << 8;
			data1 = sd_card_read(file_handle1);
			bmp_header1.important_color = bmp_header1.important_color | data1 << 16;
			data1 = sd_card_read(file_handle1);
			bmp_header1.important_color = bmp_header1.important_color | data1 << 24;

			printf("Reading image of dimension: %d x %d\n", bmp_header1.width, bmp_header1.height);
			if (bmp_header1.height > 0) {
				printf("Positive height detected, image will be loaded as upside-down\n");
			}


			printf("Image opened: INPIC2.BMP\n");
			// BMP header
			data2 = sd_card_read(file_handle2);
			bmp_header2.magic_number = data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.magic_number = bmp_header2.magic_number | data2;

			data2 = sd_card_read(file_handle2);
			bmp_header2.file_size = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.file_size = bmp_header2.file_size | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.file_size = bmp_header2.file_size | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.file_size = bmp_header2.file_size | data2 << 24;

			// unused
			data2 = sd_card_read(file_handle2);
			data2 = sd_card_read(file_handle2);
			data2 = sd_card_read(file_handle2);
			data2 = sd_card_read(file_handle2);

			data2 = sd_card_read(file_handle2);
			bmp_header2.data_offset = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.data_offset = bmp_header2.data_offset | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.data_offset = bmp_header2.data_offset | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.data_offset = bmp_header2.data_offset | data2 << 24;

			data2 = sd_card_read(file_handle2);
			bmp_header2.header_size = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.header_size = bmp_header2.header_size | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.header_size = bmp_header2.header_size | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.header_size = bmp_header2.header_size | data2 << 24;

			data2 = sd_card_read(file_handle2);
			bmp_header2.width = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.width = bmp_header2.width | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.width = bmp_header2.width | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.width = bmp_header2.width | data2 << 24;

			data2 = sd_card_read(file_handle2);
			bmp_header2.height = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.height = bmp_header2.height | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.height = bmp_header2.height | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.height = bmp_header2.height | data2 << 24;


			data2 = sd_card_read(file_handle2);
			bmp_header2.num_plane = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.num_plane = bmp_header2.num_plane | data2 << 8;

			data2 = sd_card_read(file_handle2);
			bmp_header2.num_bits_per_pixel = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.num_bits_per_pixel = bmp_header2.num_bits_per_pixel | data2 << 8;

			data2 = sd_card_read(file_handle2);
			bmp_header2.compress_mode = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.compress_mode = bmp_header2.compress_mode | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.compress_mode = bmp_header2.compress_mode | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.compress_mode = bmp_header2.compress_mode | data2 << 24;

			data2 = sd_card_read(file_handle2);
			bmp_header2.data_size = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.data_size = bmp_header2.data_size | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.data_size = bmp_header2.data_size | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.data_size = bmp_header2.data_size | data2 << 24;

			data2 = sd_card_read(file_handle2);
			bmp_header2.hor_resolution = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.hor_resolution = bmp_header2.hor_resolution | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.hor_resolution = bmp_header2.hor_resolution | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.hor_resolution = bmp_header2.hor_resolution | data2 << 24;

			data2 = sd_card_read(file_handle2);
			bmp_header2.ver_resolution = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.ver_resolution = bmp_header2.ver_resolution | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.ver_resolution = bmp_header2.ver_resolution | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.ver_resolution = bmp_header2.ver_resolution | data2 << 24;

			data2 = sd_card_read(file_handle2);
			bmp_header2.num_color_in_palette = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.num_color_in_palette = bmp_header2.num_color_in_palette | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.num_color_in_palette = bmp_header2.num_color_in_palette | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.num_color_in_palette = bmp_header2.num_color_in_palette | data2 << 24;

			data2 = sd_card_read(file_handle2);
			bmp_header2.important_color = data2;
			data2 = sd_card_read(file_handle2);
			bmp_header2.important_color = bmp_header2.important_color | data2 << 8;
			data2 = sd_card_read(file_handle2);
			bmp_header2.important_color = bmp_header2.important_color | data2 << 16;
			data2 = sd_card_read(file_handle2);
			bmp_header2.important_color = bmp_header2.important_color | data2 << 24;

			printf("Reading image of dimension: %d x %d\n", bmp_header2.width, bmp_header2.height);
			if (bmp_header2.height > 0) {
				printf("Positive height detected, image will be loaded as upside-down\n");
			}

			printf("Image opened: INPIC3.BMP\n");
			// BMP header
			data3 = sd_card_read(file_handle3);
			bmp_header3.magic_number = data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.magic_number = bmp_header3.magic_number | data3;

			data3 = sd_card_read(file_handle3);
			bmp_header3.file_size = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.file_size = bmp_header3.file_size | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.file_size = bmp_header3.file_size | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.file_size = bmp_header3.file_size | data3 << 24;

			// unused
			data3 = sd_card_read(file_handle3);
			data3 = sd_card_read(file_handle3);
			data3 = sd_card_read(file_handle3);
			data3 = sd_card_read(file_handle3);

			data3 = sd_card_read(file_handle3);
			bmp_header3.data_offset = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.data_offset = bmp_header3.data_offset | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.data_offset = bmp_header3.data_offset | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.data_offset = bmp_header3.data_offset | data3 << 24;

			data3 = sd_card_read(file_handle3);
			bmp_header3.header_size = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.header_size = bmp_header3.header_size | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.header_size = bmp_header3.header_size | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.header_size = bmp_header3.header_size | data3 << 24;

			data3 = sd_card_read(file_handle3);
			bmp_header3.width = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.width = bmp_header3.width | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.width = bmp_header3.width | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.width = bmp_header3.width | data3 << 24;

			data3 = sd_card_read(file_handle3);
			bmp_header3.height = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.height = bmp_header3.height | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.height = bmp_header3.height | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.height = bmp_header3.height | data3 << 24;


			data3 = sd_card_read(file_handle3);
			bmp_header3.num_plane = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.num_plane = bmp_header3.num_plane | data3 << 8;

			data3 = sd_card_read(file_handle3);
			bmp_header3.num_bits_per_pixel = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.num_bits_per_pixel = bmp_header3.num_bits_per_pixel | data3 << 8;

			data3 = sd_card_read(file_handle3);
			bmp_header3.compress_mode = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.compress_mode = bmp_header3.compress_mode | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.compress_mode = bmp_header3.compress_mode | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.compress_mode = bmp_header3.compress_mode | data3 << 24;

			data3 = sd_card_read(file_handle3);
			bmp_header3.data_size = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.data_size = bmp_header3.data_size | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.data_size = bmp_header3.data_size | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.data_size = bmp_header3.data_size | data3 << 24;

			data3 = sd_card_read(file_handle3);
			bmp_header3.hor_resolution = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.hor_resolution = bmp_header3.hor_resolution | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.hor_resolution = bmp_header3.hor_resolution | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.hor_resolution = bmp_header3.hor_resolution | data3 << 24;

			data3 = sd_card_read(file_handle3);
			bmp_header3.ver_resolution = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.ver_resolution = bmp_header3.ver_resolution | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.ver_resolution = bmp_header3.ver_resolution | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.ver_resolution = bmp_header3.ver_resolution | data3 << 24;

			data3 = sd_card_read(file_handle3);
			bmp_header3.num_color_in_palette = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.num_color_in_palette = bmp_header3.num_color_in_palette | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.num_color_in_palette = bmp_header3.num_color_in_palette | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.num_color_in_palette = bmp_header3.num_color_in_palette | data3 << 24;

			data3 = sd_card_read(file_handle3);
			bmp_header3.important_color = data3;
			data3 = sd_card_read(file_handle3);
			bmp_header3.important_color = bmp_header3.important_color | data3 << 8;
			data3 = sd_card_read(file_handle3);
			bmp_header3.important_color = bmp_header3.important_color | data3 << 16;
			data3 = sd_card_read(file_handle3);
			bmp_header3.important_color = bmp_header3.important_color | data3 << 24;

			printf("Reading image of dimension: %d x %d\n", bmp_header3.width, bmp_header3.height);
			if (bmp_header3.height > 0) {
				printf("Positive height detected, image will be loaded as upside-down\n");
			}

			printf("Done reading images, sending data to SDReadQueue!\n");

			bmp_header.width = 640;
			bmp_header.height = 480;
			printf("We are here!\n");
			// This is for posting the width and height info to compute_Y_task
			return_code = OSMboxPost(ReadImageWidthMailbox, (void *)(bmp_header.width));
			alt_ucosii_check_return_code(return_code);
			return_code = OSMboxPost(ReadImageHeightMailbox, (void *)(bmp_header.height));
			alt_ucosii_check_return_code(return_code);
			
			// This is the first data for posting to the queue
			finalImageData = sd_card_read(file_handle);
			image_size = 1;

			file_opened = 1;
			printf("file_opened is %d\n", file_opened);
		}
		
		// =640*480*3? check if the value is actually
		int totalDataSize = bmp_header.data_size+bmp_header1.data_size+bmp_header2.data_size+bmp_header3.data_size;
		totalDataSize = 640*480*3;
		//totalDataSize = 230400 + 230400 + 230400 + 230400 = 640(*480*3
		//this while loop is the arrow in diagram from SD_read() to SDReadQueue
		while (((return_code = OSQPost(SDReadQueue, (void *)(finalImageData & 0xFF))) == OS_NO_ERR) && (image_size < totalDataSize)){
			if(image_size < (460800)){ //top half, 460800 bytes of data
				if(image_size%(640*3) < ((640*3)/2)){ //left half
					finalImageData = sd_card_read(file_handle); //read one of the three subpixel
					image_size++;
				}else{	//right half
					finalImageData = sd_card_read(file_handle1); //read one of the three subpixel
					image_size++;
				}
			}else{	//bottom half, 921600 bytes of data
				if(image_size%(640*3) < ((640*3)/2)){ //left half
					finalImageData = sd_card_read(file_handle2); //read one of the three subpixel
					image_size++;
				}else{	//right half
					finalImageData = sd_card_read(file_handle3); //read one of the three subpixel
					image_size++;
				}
			}
		}
		//printf("my image_size before if statement is: %d\n", image_size);
		/*if(image_size==129){
				OSTimeDlyHMSM(0,0,10,0);
		}*/
		if (image_size == 640*480*3) {
			sd_card_fclose(file_handle);
			sd_card_fclose(file_handle1);
			sd_card_fclose(file_handle2);
			sd_card_fclose(file_handle3);
			printf("Done reading image3: (%d bytes)\n", bmp_header3.data_size);
			printf("Total Image Data sent was: (%d bytes)\n", image_size);
			file_opened = 0;
		}
		
		// printf("RQ filled (%d bytes)\n", image_size);
		return_code = OSMutexPost(SDMutex);
		alt_ucosii_check_return_code(return_code);
		
		OSFlagPost(SDCardFlag, SD_READ_NEED_FILL, OS_FLAG_CLR, &return_code);
		alt_ucosii_check_return_code(return_code);
		OSFlagPost(SDCardFlag, SD_READ_DATA_READY, OS_FLAG_SET, &return_code);
		alt_ucosii_check_return_code(return_code);
	}
}

void SD_write_task(void *pdata) {
	INT8U return_code = OS_NO_ERR;
	short int file_handle;
	unsigned short int data, dataR, dataG, dataB;
	int file_opened = 0;
	int file_number = 0;
	char filename[13];// = "outputFile.bmp";
	bmp_header_struct bmp_header;
	int status;
	int image_size;
	int image_width, image_height;
	
	printf("We are in SD_write_task()!\n");

	while (1) {
		OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG | SD_WRITE_DATA_READY, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
		alt_ucosii_check_return_code(return_code);
				
		// printf("W: getting SDMutex\n");
		OSMutexPend(SDMutex, 0, &return_code);
		alt_ucosii_check_return_code(return_code);
		
		if (file_opened == 0) {
			sprintf(filename, "OUTPIC%d.bmp", file_number++);
            
			while ((file_handle = sd_card_fopen(filename, 1)) < 0) {
				sprintf(filename, "OUTPIC%d.bmp", file_number++);
			}

			file_opened = 1;
			
			printf("Saving image to file (%s)...\n", filename);
            
			bmp_header.width = (int)OSMboxPend(WriteImageWidthMailbox, 0, &return_code);
			alt_ucosii_check_return_code(return_code);
			bmp_header.height = (int)OSMboxPend(WriteImageHeightMailbox, 0, &return_code);
			alt_ucosii_check_return_code(return_code);
			
			image_width = bmp_header.width;
			if (bmp_header.height < 0) image_height = -bmp_header.height;
			else image_height = bmp_header.height;
			bmp_header.width = 640;
			bmp_header.height = 480;
			image_width = 640;
			image_height = 480;
			
			// BMP header
			bmp_header.magic_number = 'B' << 8 | 'M';
			bmp_header.file_size = image_height * image_width * 3 + 54;
			bmp_header.data_offset = 54;
			bmp_header.header_size = 40;
			// bmp_header.width = 640;
			// bmp_header.height = -480;
			bmp_header.num_plane = 1;
			bmp_header.num_bits_per_pixel = 24;
			bmp_header.compress_mode = 0;
			bmp_header.data_size = image_height * image_width * 3;
			bmp_header.hor_resolution = 0;
			bmp_header.ver_resolution = 0;
			bmp_header.num_color_in_palette = 0;
			bmp_header.important_color = 0;

			status = sd_card_write(file_handle, (char)((bmp_header.magic_number >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.magic_number) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.file_size) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.file_size >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.file_size >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.file_size >> 24) & 0xFF));

			// unused
			status = sd_card_write(file_handle, 0);
			status = sd_card_write(file_handle, 0);
			status = sd_card_write(file_handle, 0);
			status = sd_card_write(file_handle, 0);

			status = sd_card_write(file_handle, (char)((bmp_header.data_offset) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.data_offset >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.data_offset >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.data_offset >> 24) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.header_size) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.header_size >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.header_size >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.header_size >> 24) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.width) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.width >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.width >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.width >> 24) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.height) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.height >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.height >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.height >> 24) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.num_plane) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.num_plane >> 8) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.num_bits_per_pixel) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.num_bits_per_pixel >> 8) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.compress_mode) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.compress_mode >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.compress_mode >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.compress_mode >> 24) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.data_size) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.data_size >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.data_size >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.data_size >> 24) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.hor_resolution) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.hor_resolution >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.hor_resolution >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.hor_resolution >> 24) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.ver_resolution) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.ver_resolution >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.ver_resolution >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.ver_resolution >> 24) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.num_color_in_palette) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.num_color_in_palette >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.num_color_in_palette >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.num_color_in_palette >> 24) & 0xFF));

			status = sd_card_write(file_handle, (char)((bmp_header.important_color) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.important_color >> 8) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.important_color >> 16) & 0xFF));
			status = sd_card_write(file_handle, (char)((bmp_header.important_color >> 24) & 0xFF));

			image_size = 0;
		} else {
			image_width = (int)OSMboxAccept(WriteImageWidthMailbox);
			image_height = (int)OSMboxAccept(WriteImageHeightMailbox);
			image_width = 640;
			image_height = 480;
		}

		do {
			// 3 data


			dataB = (unsigned short int)OSQAccept(SDWriteQueueB, &return_code);

			dataG = (unsigned short int)OSQAccept(SDWriteQueueG, &return_code);
			dataR = (unsigned short int)OSQAccept(SDWriteQueueR, &return_code);

			//data = (unsigned short int)OSQAccept(SDWriteQueue, &return_code);
			if (return_code == OS_NO_ERR) {




				status = sd_card_write(file_handle, (char)(dataR & 0xFF)); //R
				status = sd_card_write(file_handle, (char)(dataG & 0xFF)); //G
				status = sd_card_write(file_handle, (char)(dataB & 0xFF)); //B
				//image_size++;
				//image_size+=3;

				// 3 status
				image_size+=3;
			}
		} while ((return_code == OS_NO_ERR) && (image_size < 3*640*480)); //bmp_header.data_size
				
		if (image_size == 3*640*480) { //bmp_header.data_size
			file_opened = 0;
			sd_card_fclose(file_handle);

			printf("Writing image done (%d bytes)\n", image_size);
		}
		// printf("WQ emptied (%d bytes)\n", image_size);
		
		return_code = OSMutexPost(SDMutex);
		alt_ucosii_check_return_code(return_code);
		
		OSFlagPost(SDCardFlag, SD_WRITE_DATA_READY, OS_FLAG_CLR, &return_code);
		alt_ucosii_check_return_code(return_code);
		
		OSFlagPost(SDCardFlag, SD_WRITE_NEED_FILL, OS_FLAG_SET, &return_code);
		alt_ucosii_check_return_code(return_code);
	}
}

void compute_Y_task(void *pdata) {
	INT8U return_code = OS_NO_ERR;
	alt_u8 *R_val, *G_val, *B_val, *Y_val;
	alt_u8 *R_temp, *G_temp, *B_temp, *Y_temp;
	unsigned short int data;
	int i, j, RGB_count;
	int image_height, image_width;
	printf("We are in compute_Y_task()!\n");
	while (1) {
		OSSemPend(PBSemaphore[0], 0, &return_code);
		alt_ucosii_check_return_code(return_code);					
		
		printf("Compute Y task start\n");
		
		OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
		alt_ucosii_check_return_code(return_code);
		
		printf("SD card present and filesystem ok\n");

		// get the first queue of data
		OSFlagPost(SDCardFlag, SD_READ_DATA_READY, OS_FLAG_CLR, &return_code);
		alt_ucosii_check_return_code(return_code);

		OSFlagPost(SDCardFlag, SD_READ_NEED_FILL, OS_FLAG_SET, &return_code);
		alt_ucosii_check_return_code(return_code);
		
		image_width = (int)OSMboxPend(ReadImageWidthMailbox, 0, &return_code);
		alt_ucosii_check_return_code(return_code);
		image_height = (int)OSMboxPend(ReadImageHeightMailbox, 0, &return_code);
		alt_ucosii_check_return_code(return_code);
		image_width = 640;
		image_height = 480;
		
		// get memory block
		/*Y_val = OSMemGet(MemoryPartition, &return_code);
		alt_ucosii_check_return_code(return_code);*/


		// create RGB temp outside
		R_temp = OSMemGet(MemoryPartition, &return_code); //should be outside?
		alt_ucosii_check_return_code(return_code);
		G_temp = OSMemGet(MemoryPartition, &return_code);
		alt_ucosii_check_return_code(return_code);
		B_temp = OSMemGet(MemoryPartition, &return_code);
		alt_ucosii_check_return_code(return_code);

		for (i = 0; i < ((image_height < 0) ? -image_height : image_height); i++) {

			R_val = OSMemGet(MemoryPartition, &return_code); //should be outside?
			alt_ucosii_check_return_code(return_code);
			G_val = OSMemGet(MemoryPartition, &return_code);
			alt_ucosii_check_return_code(return_code);
			B_val = OSMemGet(MemoryPartition, &return_code);
			alt_ucosii_check_return_code(return_code);


			for (j = 0; j < image_width; j++) {
				for (RGB_count = 0; RGB_count < 3; ) {
					data = (unsigned short int)OSQAccept(SDReadQueue, &return_code);
				
					if (return_code == OS_Q_EMPTY) {
						// printf("Need to fill ReadQueue\n");
						OSFlagPost(SDCardFlag, SD_READ_DATA_READY, OS_FLAG_CLR, &return_code);
						alt_ucosii_check_return_code(return_code);
						OSFlagPost(SDCardFlag, SD_READ_NEED_FILL, OS_FLAG_SET, &return_code);
						alt_ucosii_check_return_code(return_code);
						OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG | SD_READ_DATA_READY, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
						alt_ucosii_check_return_code(return_code);	
					} else {
						switch (RGB_count) {
							case 0: B_val[j] = (char)(data & 0xFF); break;
							case 1: G_val[j] = (char)(data & 0xFF); break;
							case 2: R_val[j] = (char)(data & 0xFF); break;
						}
						RGB_count++;
					}
				}
				if(i<240){ //due to the order of the reading of the image (bottom up), changed the filter equation ordering accordingly
					if(j<320){
						//Original
						//negative GS* now
						B_val[j] = 255 - ((1052*R_val[j] + 2064*G_val[j] + 401*B_val[j]) >> 12);
						G_val[j] = 255 - ((1052*R_val[j] + 2064*G_val[j] + 401*B_val[j]) >> 12);
						R_val[j] = 255 - ((1052*R_val[j] + 2064*G_val[j] + 401*B_val[j]) >> 12);
						/*B_val[j] = B_val[j];
						G_val[j] = G_val[j];
						R_val[j] = R_val[j];*/


					}else{
						//Grayscale
						//negative original* now
						/*B_val[j] = B_val[j];
						G_val[j] = G_val[j];
						R_val[j] = R_val[j];*/
						B_val[j] = 255 - B_val[j];
						G_val[j] = 255 - G_val[j];
						R_val[j] = 255 - R_val[j];
					}
				}else{
					if(j<320){
						//Negative of Grayscale
						//original* now
						B_val[j] = B_val[j];
						G_val[j] = G_val[j];
						R_val[j] = R_val[j];

					}else{
						//Negative of Original
						//GS* now
						/*B_val[j] = B_val[j];
						G_val[j] = G_val[j];
						R_val[j] = R_val[j];*/
						B_val[j] = (1052*R_val[j] + 2064*G_val[j] + 401*B_val[j]) >> 12;
						G_val[j] = (1052*R_val[j] + 2064*G_val[j] + 401*B_val[j]) >> 12;
						R_val[j] = (1052*R_val[j] + 2064*G_val[j] + 401*B_val[j]) >> 12;
					}
				}
				//Y_val[j] = (1052*R_val[j] + 2064*G_val[j] + 401*B_val[j]) >> 12;  //implement filter equation here
			//decide when to send RGB or YYY depending on region 0,1,2,3
			}
			/*return_code = OSMemPut(MemoryPartition, (void *)R_val);
			alt_ucosii_check_return_code(return_code);
			return_code = OSMemPut(MemoryPartition, (void *)G_val);
			alt_ucosii_check_return_code(return_code);
			return_code = OSMemPut(MemoryPartition, (void *)B_val);
			alt_ucosii_check_return_code(return_code);*/

			// One line of Y is ready
			seg7_show(SEG7_DISPLAY_0_BASE, i);
			// printf("line %d of Y computed\n", i);
			return_code = OSMboxPost(YImageWidthMailbox, (void *)(image_width));
			alt_ucosii_check_return_code(return_code);
			return_code = OSMboxPost(YImageHeightMailbox, (void *)(image_height));
			alt_ucosii_check_return_code(return_code);
			/*return_code = OSMboxPost(YMailbox, (void *)(Y_val));
			alt_ucosii_check_return_code(return_code);*/
			return_code = OSMboxPost(RMailbox, (void *)(R_temp));
			alt_ucosii_check_return_code(return_code);
			return_code = OSMboxPost(GMailbox, (void *)(G_temp));
			alt_ucosii_check_return_code(return_code);
			return_code = OSMboxPost(BMailbox, (void *)(B_temp));
			alt_ucosii_check_return_code(return_code);
			
			OSFlagPost(SDCardFlag, SD_PROCESS_Y_READY, OS_FLAG_CLR, &return_code);
			alt_ucosii_check_return_code(return_code);

			OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG | SD_PROCESS_Y_READY, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
			alt_ucosii_check_return_code(return_code);

			//free temp RGB either here or
			//****************
			return_code = OSMemPut(MemoryPartition, (void *)R_temp);
			alt_ucosii_check_return_code(return_code);
			return_code = OSMemPut(MemoryPartition, (void *)G_temp);
			alt_ucosii_check_return_code(return_code);
			return_code = OSMemPut(MemoryPartition, (void *)B_temp);
			alt_ucosii_check_return_code(return_code);
		}

		printf("Compute Y done\n");
		/*return_code = OSMemPut(MemoryPartition, (void *)Y_val);
		alt_ucosii_check_return_code(return_code);*/

		/*return_code = OSMemPut(MemoryPartition, (void *)R_temp);
		alt_ucosii_check_return_code(return_code);
		return_code = OSMemPut(MemoryPartition, (void *)G_temp);
		alt_ucosii_check_return_code(return_code);
		return_code = OSMemPut(MemoryPartition, (void *)B_temp);
		alt_ucosii_check_return_code(return_code);*/

		return_code = OSMemPut(MemoryPartition, (void *)R_val);
		alt_ucosii_check_return_code(return_code);
		return_code = OSMemPut(MemoryPartition, (void *)G_val);
		alt_ucosii_check_return_code(return_code);
		return_code = OSMemPut(MemoryPartition, (void *)B_val);
		alt_ucosii_check_return_code(return_code);

		// or free temp RGB here
		//****************
		/*return_code = OSMemPut(MemoryPartition, (void *)R_temp);
		alt_ucosii_check_return_code(return_code);
		return_code = OSMemPut(MemoryPartition, (void *)G_temp);
		alt_ucosii_check_return_code(return_code);
		return_code = OSMemPut(MemoryPartition, (void *)B_temp);
		alt_ucosii_check_return_code(return_code);*/
	}
}

void process_Y_task(void *pdata) {
	INT8U return_code = OS_NO_ERR;
	alt_u8 *Y_val,*R_val,*G_val,*B_val;
	int i,j;
	int didItSend = 0;
	unsigned short int data;
	int image_width, image_height;
	printf("We are in process_Y_task()!\n");
	while (1) {
		/*Y_val = (alt_u8 *)OSMboxPend(YMailbox, 0, &return_code);
		alt_ucosii_check_return_code(return_code);*/




		R_val = (alt_u8 *)OSMboxPend(RMailbox, 0, &return_code);
				alt_ucosii_check_return_code(return_code);
		G_val = (alt_u8 *)OSMboxPend(GMailbox, 0, &return_code);
				alt_ucosii_check_return_code(return_code);
		B_val = (alt_u8 *)OSMboxPend(BMailbox, 0, &return_code);
				alt_ucosii_check_return_code(return_code);

		OSFlagPost(SDCardFlag, SD_WRITE_DATA_READY, OS_FLAG_CLR, &return_code);
		alt_ucosii_check_return_code(return_code);

		image_width = (int)OSMboxPend(YImageWidthMailbox, 0, &return_code);
		image_width = 640;
		alt_ucosii_check_return_code(return_code);
		image_height = (int)OSMboxPend(YImageHeightMailbox, 0, &return_code);
		alt_ucosii_check_return_code(return_code);
		image_height = 480;

		return_code = OSMboxPost(WriteImageWidthMailbox, (void *)(image_width));
		alt_ucosii_check_return_code(return_code);
		return_code = OSMboxPost(WriteImageHeightMailbox, (void *)(image_height));
		alt_ucosii_check_return_code(return_code);
		
		OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
		alt_ucosii_check_return_code(return_code);
		

		for (i = 0; i < image_width; ) {
			/*if (OSQPost(SDWriteQueue, (void *)(Y_val[i] & 0xFF)) == OS_Q_FULL) {
				// printf("WriteQueue full\n");
				OSFlagPost(SDCardFlag, SD_WRITE_NEED_FILL, OS_FLAG_CLR, &return_code);
				alt_ucosii_check_return_code(return_code);
				OSFlagPost(SDCardFlag, SD_WRITE_DATA_READY, OS_FLAG_SET, &return_code);
				alt_ucosii_check_return_code(return_code);
				OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG | SD_WRITE_NEED_FILL, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
				alt_ucosii_check_return_code(return_code);
			} else {
				i++;
			}*/
			// RGB write queue
			if ((OSQPost(SDWriteQueueB, (void *)(B_val[i] & 0xFF)) != OS_Q_FULL) && (OSQPost(SDWriteQueueG, (void *)(G_val[i] & 0xFF)) != OS_Q_FULL)
					&& (OSQPost(SDWriteQueueR, (void *)(R_val[i] & 0xFF)) != OS_Q_FULL)){
						i++; //only increment the queue pointer long as none of the queues are full
			}else{ //otherwise if full, signal OS to empty the queue
						OSFlagPost(SDCardFlag, SD_WRITE_NEED_FILL, OS_FLAG_CLR, &return_code);
						alt_ucosii_check_return_code(return_code);
						OSFlagPost(SDCardFlag, SD_WRITE_DATA_READY, OS_FLAG_SET, &return_code);
						alt_ucosii_check_return_code(return_code);
						OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG | SD_WRITE_NEED_FILL, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
						alt_ucosii_check_return_code(return_code);
			}

			/*OSQPost(SDWriteQueue, (void *)(B_val[i] & 0xFF));
			OSQPost(SDWriteQueue, (void *)(G_val[i] & 0xFF));
			if (OSQPost(SDWriteQueue, (void *)(R_val[i] & 0xFF)) == OS_Q_FULL) { //if prev two worked, check if red worked
				printf("WriteQueue full\n");
				printf("Did not send!\n");
				OSFlagPost(SDCardFlag, SD_WRITE_NEED_FILL, OS_FLAG_CLR, &return_code);
				alt_ucosii_check_return_code(return_code);
				OSFlagPost(SDCardFlag, SD_WRITE_DATA_READY, OS_FLAG_SET, &return_code);
				alt_ucosii_check_return_code(return_code);
				OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG | SD_WRITE_NEED_FILL, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
				alt_ucosii_check_return_code(return_code);
			} else {
				i++;
			}*/

		}


		// Let the SD write task to finish writing the remaining data in the queue
		OSFlagPost(SDCardFlag, SD_WRITE_NEED_FILL, OS_FLAG_CLR, &return_code);
		alt_ucosii_check_return_code(return_code);
		OSFlagPost(SDCardFlag, SD_WRITE_DATA_READY, OS_FLAG_SET, &return_code);
		alt_ucosii_check_return_code(return_code);
		OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG | SD_WRITE_NEED_FILL, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
		alt_ucosii_check_return_code(return_code);
		
		// printf("One line Y written\n");
		OSFlagPost(SDCardFlag, SD_PROCESS_Y_READY, OS_FLAG_SET, &return_code);
		alt_ucosii_check_return_code(return_code);
	}
}

// Task launcher
// It creates all the custom tasks
// And then it deletes itself
void task_launcher(void *pdata) {
	INT8U return_code = OS_NO_ERR;

	#if OS_CRITICAL_METHOD == 3
			OS_CPU_SR cpu_sr;
	#endif

	printf("Starting task launcher...\n");
	while (1) {
		OS_ENTER_CRITICAL();
		printf("Creating tasks...\n");

		return_code = OSTaskCreateExt(SD_presence_detect_task,
			NULL,
			(void *)&SD_presence_detect_stk[TASK_STACKSIZE-1],
			SD_PRESENCE_DETECT_PRIORITY,
			SD_PRESENCE_DETECT_PRIORITY,
			&SD_presence_detect_stk[0],
			TASK_STACKSIZE,
			&SD_presence_detect_tcb,
			0);
		alt_ucosii_check_return_code(return_code);

		return_code = OSTaskCreateExt(SD_read_task,
			NULL,
			(void *)&SD_read_stk[TASK_STACKSIZE-1],
			SD_READ_PRIORITY,
			SD_READ_PRIORITY,
			&SD_read_stk[0],
			TASK_STACKSIZE,
			&SD_read_tcb,
			0);
		alt_ucosii_check_return_code(return_code);

		return_code = OSTaskCreateExt(SD_write_task,
			NULL,
			(void *)&SD_write_stk[TASK_STACKSIZE-1],
			SD_WRITE_PRIORITY,
			SD_WRITE_PRIORITY,
			&SD_write_stk[0],
			TASK_STACKSIZE,
			&SD_write_tcb,
			0);
		alt_ucosii_check_return_code(return_code);

		return_code = OSTaskCreateExt(compute_Y_task,
			NULL,
			(void *)&compute_Y_stk[TASK_STACKSIZE-1],
			COMPUTE_Y_PRIORITY,
			COMPUTE_Y_PRIORITY,
			&compute_Y_stk[0],
			TASK_STACKSIZE,
			&compute_Y_tcb,
			0);
		alt_ucosii_check_return_code(return_code);

		return_code = OSTaskCreateExt(process_Y_task,
			NULL,
			(void *)&process_Y_stk[TASK_STACKSIZE-1],
			PROCESS_Y_PRIORITY,
			PROCESS_Y_PRIORITY,
			&process_Y_stk[0],
			TASK_STACKSIZE,
			&process_Y_tcb,
			0);
		alt_ucosii_check_return_code(return_code);

		printf("Finish creating tasks...\n");

		printf("\n");
		OSTimeDlyHMSM(0, 0, 1, 0);

		return_code = OSTaskDel(OS_PRIO_SELF);
		alt_ucosii_check_return_code(return_code);

		OS_EXIT_CRITICAL();
	}
}
