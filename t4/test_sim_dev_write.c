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

int fd;
int ioctl_control_data = 0XABCDEFEF;
int ioctl_status_data = 0x00000000;
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

void build_random_string(char *str, int size) {
	char random_char;
	int i;
	for (i = 0; i < size; i++) {
		// average english word length is about 5 characters, so we set a 1/6 chance to generate a space in the random string
		if (rand() % 5 == 0) {
			random_char = ' ';
		}
		else {
			random_char = (rand() % 58) + 65;
			while (random_char > 90 && random_char < 97) { // range of non-alphanumeric ascii characters
				random_char = (char)(rand() % 58) + 65;
			}
		}
		str[i] = random_char;
	}
}

void timer_callback(int sig)
{
	if (rand() % 2 == 0) { // 50% chance to perform a write operation at each timer callback

		// get time of operation	
		time_t curr_time = time(0);
		char time_buff[20];
		struct tm *timeinfo;
		timeinfo = localtime(&curr_time);
		strftime(time_buff, sizeof(time_buff), "%b %d %H:%M:%S", timeinfo);

		// generate random string with random size
		int num_of_sectors = rand() % NUM_OF_SECTS;
		int buffer_size = num_of_sectors * SECT_SIZE; 
		char buffer[buffer_size];
		build_random_string(buffer, buffer_size);

		// convert logical address given by user into physical address
		physaddr_t phys_address_struct;
		int logical_address = rand() % MAX_LOGICAL_SECTOR;
		log_to_phys(logical_address, &phys_address_struct);
		disk_control_reg->cyl = phys_address_struct.cyl;
		disk_control_reg->head = phys_address_struct.head;
		disk_control_reg->sect = phys_address_struct.sect;

		// copy the local control register into the device control register
		ioctl(fd, IOCTL_SIM_DEV_WRITE, &ioctl_control_data);

		
		printf("WRITE: TIME OF OPERATION: %s\n", time_buff);
		printf("Writing to %d sectors starting at cyl %d, head %d, and sector %d (logical_address %d)\n", 
			num_of_sectors, disk_control_reg->cyl, disk_control_reg->head, disk_control_reg->sect, logical_address);
		printf("Random content generated for writing:\n\"\n%s\n\"\n", buffer);
		
		// Write the generated data into the disk device.
		int retval = write(fd, buffer, buffer_size + 1);

		// check for write errors
		if (retval != 0) {
			// get write error code from device status register
			ioctl(fd, IOCTL_SIM_DEV_READ, &ioctl_status_data);
			switch (disk_status_reg->error_code) {
				case ECYL:  fprintf(stderr, "ERR: bad cyl\n");
							break;
				case EHEAD: fprintf(stderr, "ERR: bad head\n");
							break;
				case ESECT: fprintf(stderr, "ERR: bad sect\n");
							break;
				case ESECTCOUNT: fprintf(stderr, "ERR: bad number of sectors\n");
							break;
				case ERDY:  fprintf(stderr, "ERR: disk not ready for reading\n");
							break;
				case EINVAL: fprintf(stderr, "ERR: einval\n");
							break; 
				case EFAULT: fprintf(stderr, "ERR: efault\n");
							break;
				default:	fprintf(stderr, "ERR: unkown error occurred\n");
			}
		}
		
		printf("\n");
	}

    count--;
}

int main(void)
{
	printf("TEST_SIM_DEV_WRITE\n");
	srand(time(NULL));

	// open the simulated device 
	fd = open("/dev/sim_dev", O_WRONLY | O_SYNC);
	if( fd == -1)
	   oops("Unable to open device...", 1);

	(void) signal(SIGALRM, timer_callback);
    start_timer();
    while(count >= 0);

	close(fd);
}