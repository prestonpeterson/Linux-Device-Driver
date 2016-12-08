/**
* Name: Preston Peterson
* Lab/task: Project 2 Task 1
* Date: 12/01/16
**/

#include <math.h>
#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

disk_t disk;

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
	memset(buffer, 0, strlen((char *)buffer));

	// compute physical address:
	physaddr_t phaddr;
	log_to_phys(logical_block_num, &phaddr);
	
	// copy disk sectors to buffer
	int iteration = 0;
	int buffer_index = 0;
	int sector_index = 0;
	int i = phaddr.sect;
	for (iteration = 0; iteration < num_of_sectors; iteration++) {
		
		for (sector_index = 0; sector_index < SECT_SIZE; sector_index++) {
			((char *)buffer)[buffer_index] = disk[phaddr.cyl][phaddr.head][i][sector_index];
			if (((char *)buffer)[buffer_index] == '\0') {
				buffer_index++;
				break;
			}
			buffer_index++;
		}

		i = (i + 1) % NUM_OF_SECTS;
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
	memset(buffer, 0, strlen(buffer));
	return 0;
}


int main(int argc, char *argv[]) {

	srand(time(NULL));
	int logical_address;
	physaddr_t phys_address_struct;
	int prev = time(NULL);
	int curr;
	int total_iters = 3;
	int curr_iter = 0;
	char buffer[NUM_OF_SECTS * SECT_SIZE];
	int number_of_sectors;
	printf("WELCOME. SECTOR SIZE IS SET TO %d\n", SECT_SIZE);
	while (curr_iter < total_iters)
	{
		if ((curr = time(NULL)) != prev) { // This will trigger once per second
			logical_address = rand() % MAX_LOGICAL_SECTOR;
			printf("\n*****\nRandomly generated logical address: %d\n", logical_address);
			log_to_phys(logical_address, &phys_address_struct);
			printf("log_to_phys(%d) = Cyl: %d. Head: %d. Sect: %d\n", 
				logical_address, phys_address_struct.cyl, phys_address_struct.head, phys_address_struct.sect);			
			printf("phys_to_log returned logical address: %d\n*****\n", phys_to_log(&phys_address_struct));

			// generate "random" string for the buffer
			if (curr_iter % 3 == 0)
				strcpy(buffer, "HELLO THERE");
			else if (curr_iter % 2 == 0)
				strcpy(buffer, "123456789012345678901234567890");
			else
				strcpy(buffer, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");

			// calculate number of sectors required based on size of the contents of the buffer
			number_of_sectors = ceil(((double)strlen(buffer)) / SECT_SIZE);

			// write the buffer to the disk
			printf("Writing \"%s\" into %d sectors at address %d\n", buffer, number_of_sectors, logical_address);
			write(logical_address, number_of_sectors, (void *)buffer);
			
			// read from the first written disk sector back into the buffer
			read(logical_address, 1, (void **)&buffer);
			printf("Read \"%s\" from first sector at address %d\n", buffer, logical_address);

			prev = curr;
			curr_iter++;
		}
	}

	return 0;
}