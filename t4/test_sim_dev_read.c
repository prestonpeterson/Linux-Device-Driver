/**
* Name: Preston Peterson
* Lab/task: Project 2 Task 4
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
#include <signal.h>
#include <time.h>
#include "sim_dev.h"

#define oops(msg, errnum) { perror(msg); exit(errnum); }

#define BUFSIZE 4096

int logaddr, num_of_sectors;
int fd;
int ioctl_control_data = 0XABCDEFEF;
int ioctl_status_data;
DISK_REGISTER *disk_status_reg = (DISK_REGISTER *) &ioctl_status_data;
DISK_REGISTER *disk_control_reg = (DISK_REGISTER *) &ioctl_control_data;
extern int count;
extern timer_t gTimerid;

int log_to_phys(int logaddr, physaddr_t *phaddr) {
	phaddr->cyl = logaddr/(NUM_OF_HEADS * NUM_OF_SECTS);
	phaddr->head = (logaddr/NUM_OF_SECTS) % NUM_OF_HEADS;
	phaddr->sect = (logaddr % NUM_OF_SECTS) + 1;
	return 0;
}

void timer_callback(int sig)
{
	// poll the device to see if it is ready for reading
	ioctl(fd, IOCTL_SIM_DEV_READ, &ioctl_status_data);

	if (disk_status_reg->ready) {
		// get time of operation
		time_t curr_time = time(0);
		char time_buff[20];
		struct tm *timeinfo;
		timeinfo = localtime(&curr_time);
		strftime(time_buff, sizeof(time_buff), "%b %d %H:%M:%S", timeinfo);

		// convert user's given logical address into a physical address
		physaddr_t phys_address_struct;
		log_to_phys(logaddr, &phys_address_struct);

		// copy the physical address into the local control register
		disk_control_reg->cyl = phys_address_struct.cyl;
		disk_control_reg->head = phys_address_struct.head;
		disk_control_reg->sect = phys_address_struct.sect;
		// copy the number of sectors to read (also given by the user) to the local control register
		disk_control_reg->num_of_sectors = num_of_sectors;

		// copy the local control register into the device control register
		ioctl(fd, IOCTL_SIM_DEV_WRITE, &ioctl_control_data);
		
		// create buffer and read from the disk device
		int buffer_size = num_of_sectors * SECT_SIZE; 
		char buffer[buffer_size];
		read(fd, buffer, buffer_size + 1);
		
		printf("READ: TIME OF OPERATION %s\n", time_buff);
		printf("Content found at address %d sectors 0 - %d:\n", logaddr, num_of_sectors);
		printf("\"\n%s\n\"\n", buffer);

		// get the next logical address and number of sectors to read from the user
		printf("Enter the logical address to read from and the number of sectors to read, or enter -1 to exit.\n");
		scanf(" %d", &logaddr);
		if (logaddr == -1)
			exit(0);
		scanf(" %d", &num_of_sectors);
		if (num_of_sectors == -1)
			exit(0);
	}
	else {
		printf("Waiting for device to be ready... status = %d\n", disk_status_reg->ready);
	}

    count--;
}



int main(void)
{
	printf("TEST_SIM_DEV_READ\n");
	printf("Enter the logical address to read from and the number of sectors to read\n");
	scanf(" %d %d", &logaddr, &num_of_sectors);

	// open the simulated device 
	fd = open("/dev/sim_dev", O_RDONLY | O_SYNC);
	if( fd == -1)
	   oops("Unable to open device...", 1);

	(void) signal(SIGALRM, timer_callback);
    start_timer();
    while(count >= 0 && logaddr >= 0);

	close(fd);
}