Readme - Hello MicroC/OS-II Hello Software Example

Hello_uosii is a simple hello world program running MicroC/OS-II.  The
purpose of the design is to be a very simple application that just
demonstrates MicroC/OS-II running on NIOS II.  The design doesn't account
for issues such as checking system call return codes. etc.

				if(colCounter<320){//read 320 from image 0
					printf("IMAGE0 %d\n",colCounter );
					finalImageData = sd_card_read(file_handle); //read one of the three subpixel
					pixelCounter++;
					image_size++;
					if(pixelCounter==3){
						pixelCounter = 0;
						colCounter++;
					}
				}else if(colCounter<640){
					printf("IMAGE1 %d\n",colCounter );
					finalImageData = sd_card_read(file_handle1); //read one of the three subpixel
					pixelCounter++;
					image_size++;
					if(pixelCounter==3){
						pixelCounter = 0;
						colCounter++;
					}
					if(colCounter == 640){
						colCounter=0;
						rowCounter++;
					}
				}

				if(image_size == 460800){
					printf("RESETTING!!\n");
					colCounter=0;
					rowCounter=0;
					pixelCounter=0;
					sd_card_fclose(file_handle);
					sd_card_fclose(file_handle1);
				}


			}else if(image_size < 921600){
				//printf("..");
				//bottom half, 921600 bytes of data
				if(colCounter<320){//read 320 from image 0
					printf("IMAGE2 %d\n",colCounter );
					finalImageData = sd_card_read(file_handle2);
					pixelCounter++;
					image_size++;
					if(pixelCounter==3){
						pixelCounter = 0;
						colCounter++;
					}
				}else if(colCounter<640){
					printf("IMAGE3 %d\n",colCounter );
					finalImageData = sd_card_read(file_handle3); //read one of the three subpixel
					pixelCounter++;
					image_size++;
					if(pixelCounter==3){
						pixelCounter = 0;
						colCounter++;
					}
					if(colCounter == 640){
						colCounter=0;
						rowCounter++;
					}
				}
				if(image_size >= totalDataSize){
					printf("Closed all files\n");
					colCounter=0;
					rowCounter=0;
					pixelCounter=0;
					sd_card_fclose(file_handle2);
					sd_card_fclose(file_handle3);
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
			OSQPost(SDWriteQueue, (void *)(B_val[i] & 0xFF));
			OSQPost(SDWriteQueue, (void *)(G_val[i] & 0xFF));
			if (OSQPost(SDWriteQueue, (void *)(R_val[i] & 0xFF)) == OS_Q_FULL) { //if prev two worked, check if red worked
				// printf("WriteQueue full\n");
				///printf("Did not send!\n");
				OSFlagPost(SDCardFlag, SD_WRITE_NEED_FILL, OS_FLAG_CLR, &return_code);
				alt_ucosii_check_return_code(return_code);
				OSFlagPost(SDCardFlag, SD_WRITE_DATA_READY, OS_FLAG_SET, &return_code);
				alt_ucosii_check_return_code(return_code);
				OSFlagPend(SDCardFlag, SD_PRESENCE_FLAG | SD_FILESYSTEM_FLAG | SD_WRITE_NEED_FILL, OS_FLAG_WAIT_SET_ALL, 0, &return_code);
				alt_ucosii_check_return_code(return_code);
			} else {
				i++;
			}

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