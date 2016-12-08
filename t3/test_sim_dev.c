/**
* Name: Preston Peterson
* Lab/task: Project 2 Task 3
* Date: 12/08/16
**/

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <linux/errno.h>

#include "sim_dev.h"

#define oops(msg, errnum) { perror(msg); exit(errnum); }

#define BUFSIZE 4096

int log_to_phys(int logaddr, physaddr_t *phaddr) {
	phaddr->cyl = logaddr/(NUM_OF_HEADS * NUM_OF_SECTS);
	phaddr->head = (logaddr/NUM_OF_SECTS) % NUM_OF_HEADS;
	phaddr->sect = (logaddr % NUM_OF_SECTS) + 1;
	return 0;
}

int main(void)
{
	int fd, ret_val, wlen, tmp, tmp2;
	
	// test message to send to the device
	char msg[2][128] = {"Skeleton Kernel Module Test", "Nick had a Bick with a long Wick"};
	char receive_buffer[128];
	
	// variables for holding device control data
	int ioctl_control_data = 0XABCDEFEF;
	int ioctl_status_data = 0x00000000;
	DISK_REGISTER *disk_status_reg = (DISK_REGISTER *) &ioctl_status_data;
	DISK_REGISTER *disk_control_reg = (DISK_REGISTER *) &ioctl_control_data;

	// set local control register bit fields
	int starting_sector = 2;
	disk_control_reg->ready = 1;
	disk_control_reg->cyl = 2;
	disk_control_reg->head = 2;
	disk_control_reg->sect = starting_sector;
	disk_control_reg->num_of_sectors = 0;

   // open the I/O channel to the device
	fd = open("/dev/sim_dev", O_RDWR | O_SYNC);
	if( fd == -1)
	   oops("Unable to open device...", 1);

	int i;
	for (i = 0; i < 2; i++) {
		// increment target sector
		disk_control_reg->sect = disk_control_reg->sect + 2;
		disk_control_reg->num_of_sectors = disk_control_reg->num_of_sectors + 1;
		// set disk control register
		ioctl(fd, IOCTL_SIM_DEV_WRITE, &ioctl_control_data);
		// test device write function
		ret_val = write(fd, msg[i], strlen(msg[i]) + 1);
		switch (ret_val) {
			case ECYL: printf("bad cyl\n");
			case EHEAD: printf("bad head\n");
			case ESECT: printf("bad sect\n");
			case ESECTCOUNT: fprintf(stderr, "bad number of sectors\n");
			case ERDY: printf("disk not ready for writing\n");
			case -EINVAL: printf("einval\n"); 
			case -EFAULT: printf("efault\n");
		}
		printf("'%s' wrote to /dev/sim_dev\n", msg[i]);

		// now read the location that was written to, to ensure the write was successful
		read(fd, receive_buffer, 128);
		ioctl(fd, IOCTL_SIM_DEV_READ, &ioctl_status_data);
		switch (disk_status_reg->error_code) {
			case ECYL: printf("bad cyl\n");
			case EHEAD: printf("bad head\n");
			case ESECT: printf("bad sect\n");
			case ESECTCOUNT: fprintf(stderr, "bad number of sectors\n");
			case ERDY: printf("disk not ready for reading\n");
			case EINVAL: printf("einval\n"); 
			case EFAULT: printf("efault\n");
		}
		printf("'%s' read from /dev/sim_dev\n", receive_buffer);
	}

	// now try reading all of the previously written sectors in one read
	disk_control_reg->sect = starting_sector;
	disk_control_reg->num_of_sectors = 10;
	ioctl(fd, IOCTL_SIM_DEV_WRITE, &ioctl_control_data);
	read(fd, receive_buffer, 128);

	printf("'%s' read from /dev/sim_dev\n", receive_buffer);

	close(fd);
}