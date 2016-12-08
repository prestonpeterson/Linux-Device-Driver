/**
* Name: Preston Peterson
* Lab/task: Project 2 Task 2
* Date: 12/04/16
**/

#include <math.h>
#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

disk_t disk;
extern timer_t gTimerid;
extern int count;

int log_to_phys(int logaddr, physaddr_t *phaddr) {
	phaddr->cyl = logaddr/(NUM_OF_HEADS * NUM_OF_SECTS);
	phaddr->head = (logaddr/NUM_OF_SECTS) % NUM_OF_HEADS;
	phaddr->sect = (logaddr % NUM_OF_SECTS) + 1;
	return 0;
}

int phys_to_log(physaddr_t *phaddr) {
	return (phaddr->cyl * NUM_OF_HEADS + phaddr->head) * NUM_OF_SECTS + (phaddr->sect - 1);
}

int read(int logical_block_num, int num_of_sectors, void **buffer) {
	// parameter validation:
	if ((logical_block_num < 0 || logical_block_num >= MAX_LOGICAL_SECTOR)
		|| (num_of_sectors < 0 || num_of_sectors > NUM_OF_SECTS)) 
	{
		fprintf(stderr, "ERROR: OUT OF BOUNDS ARGUMENT TO READ\n");
		return -1;
	}
	// clear buffer
	memset(buffer, 0, strlen((char *)buffer));

	// compute physical address:
	physaddr_t phaddr;
	log_to_phys(logical_block_num, &phaddr);
	
	// copy disk sectors to buffer
	int sector = 0;
	int buffer_index = 0;
	int sector_index = 0;
	int sect = phaddr.sect;
	while (sector < num_of_sectors) {
		for (sector_index = 0; sector_index < SECT_SIZE; sector_index++) {
			((char *)buffer)[buffer_index] = disk[phaddr.cyl][phaddr.head][sect][sector_index];
			if (((char *)buffer)[buffer_index] == '\0') {
				buffer_index++;
				break;
			}
			buffer_index++;
		}

		sector++;
		sect = (sect + 1) % NUM_OF_SECTS;
	}
	return 0;
}

int write(int logical_block_num, int num_of_sectors, void *buffer) {
	// parameter validation:
	if ((logical_block_num < 0 || logical_block_num >= MAX_LOGICAL_SECTOR)
		|| (num_of_sectors < 0 || num_of_sectors > NUM_OF_SECTS)) 
	{
		fprintf(stderr, "ERROR: OUT OF BOUNDS ARGUMENT TO WRITE\n");
		return -1;
	}

	// compute physical address:
	physaddr_t phaddr;
	log_to_phys(logical_block_num, &phaddr);

	// copy buffer to disk
	int sector_index = 0;
	int buffer_index = 0;
	int i = phaddr.sect;
	int sector = 0;

	while (sector < num_of_sectors) {
		for (sector_index = 0; sector_index < SECT_SIZE; sector_index++) {
			disk[phaddr.cyl][phaddr.head][i][sector_index] = ((char *)buffer)[buffer_index];
			if (((char *)buffer)[buffer_index] == '\0') {
				buffer_index++;
				break;
			}
			buffer_index++;
		}
		sector++;
		i = (i + 1) % NUM_OF_SECTS;
	}
	// clear buffer
	memset(buffer, 0, strlen((char *)buffer));
	return 0;
}

void build_random_string(char *str, int size) {
	char random_char;
	int i;
	for (i = 0; i < size; i++) {
		// generate a space sometimes
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
	if (rand() % 2 == 0) { // 50% chance to take an action at each timer callback
		int num_of_sectors = rand() % NUM_OF_SECTS;
		int buffer_size = num_of_sectors * SECT_SIZE; 
		char buffer[buffer_size];
		physaddr_t phys_address_struct;
		int logical_address = rand() % MAX_LOGICAL_SECTOR;
		log_to_phys(logical_address, &phys_address_struct);

		// get time of operation
		time_t curr_time = time(0);
		char buff[20];
		struct tm *timeinfo;
		timeinfo = localtime(&curr_time);
		strftime(buff, sizeof(buff), "%b %d %H:%M:%S", timeinfo);

		int read_action = rand() % 2;
		if (read_action) {
			printf("READING %d SECTORS FROM LOGICAL ADDRESS %d...\n", num_of_sectors, logical_address);
			printf("TIME OF OPERATION: %s\n", buff);
			printf("log_to_phys(%d) = Cyl: %d. Head: %d. Sect: %d\n", 
				logical_address, phys_address_struct.cyl, phys_address_struct.head, phys_address_struct.sect);
			read(logical_address, num_of_sectors, (void **)&buffer);
			printf("Read \n\"%s\n\" from first %d sectors at address %d\n", buffer, num_of_sectors, logical_address);
		}
		else {
			// put random string into buffer
			build_random_string(buffer, buffer_size);
			printf("WRITING %d SECTORS TO LOGICAL ADDRESS %d...\n", num_of_sectors, logical_address);
			printf("TIME OF OPERATION: %s\n", buff);
			printf("log_to_phys(%d) = Cyl: %d. Head: %d. Sect: %d\n", 
				logical_address, phys_address_struct.cyl, phys_address_struct.head, phys_address_struct.sect);
			
			printf("Random content generated for writing:\n\"\n%s\n\"\n", buffer);
			
			write(logical_address, num_of_sectors, (void *)buffer);
		}
		printf("\n");
	}

    count--;
}


int main(int argc, char *argv[]) {

	srand(time(NULL));
	printf("WELCOME. SECTOR SIZE IS SET TO %d\n", SECT_SIZE);

	(void) signal(SIGALRM, timer_callback);
    start_timer();
    while(count >= 0);

	return 0;
}