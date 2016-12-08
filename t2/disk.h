/**
* Name: Preston Peterson
* Lab/task: Project 2 Task 2
* Date: 12/04/16
**/

#ifndef __DISK_H
#define __DISH_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

#define SECT_SIZE 20
#define NUM_OF_SECTS 128
#define NUM_OF_CYLS 200
#define NUM_OF_HEADS 10

#define MAX_LOGICAL_SECTOR 256000 // NUM_OF_HEADS * NUM_OF_CYLS * NUM_OF_SECTS

typedef char sector_t[SECT_SIZE];
typedef sector_t head_t[NUM_OF_SECTS];
typedef head_t cylinder_t[NUM_OF_HEADS];
typedef cylinder_t disk_t[NUM_OF_CYLS];

typedef struct
{
  int cyl;
  int head;
  int sect;
} physaddr_t;

/**
This function translates a logical to a physical address.
**/
int log_to_phys(int logaddr, physaddr_t *phaddr);

/**
This function translates a physical to a logical address.
**/
int phys_to_log(physaddr_t *phaddr);

/**
This function copies num_of_sectors sectors from the disk to the buffer starting with sector pointed to by logical_block_num. You should validate the parameters before serving the request.
**/
int read(int logical_block_num, int num_of_sectors, void **buffer);

/**
This function copies num_of_sectors sectors from the buffer to the disk starting at sector pointed to by logical_block_num. Here, you also should validate the parameters.
**/
int write(int logical_block_num, int num_of_sectors, void *buffer);

void start_timer(void);
void stop_timer(void);
void timer_callback(int);

#endif
