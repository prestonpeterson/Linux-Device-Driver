/**
* Name: Preston Peterson
* Lab/task: Project 2 Task 3
* Date: 12/08/16
**/

#ifndef __SIM_DEV_H_
#define __SIM_DEV_H_

#define IOCTL_SIM_DEV_WRITE _IOR(0, 1, int)
#define IOCTL_SIM_DEV_READ _IOR(0, 2, int)

#define ECYL 1
#define EHEAD 2
#define ESECT 3
#define ESECTCOUNT 4
#define ERDY 5

#define SECT_SIZE 16
#define NUM_OF_SECTS 128
#define NUM_OF_CYLS 200
#define NUM_OF_HEADS 10

#define MAX_LOGICAL_SECTOR 256000 // NUM_OF_HEADS * NUM_OF_CYLS * NUM_OF_SECTS

typedef char sect_t[SECT_SIZE];
typedef sect_t head_t[NUM_OF_SECTS];
typedef head_t cylinder_t[NUM_OF_HEADS];
typedef cylinder_t disk_t[NUM_OF_CYLS];

typedef struct
{
  int cyl;
  int head;
  int sect;
} physaddr_t;

typedef struct {
     unsigned ready:1;
     unsigned error_code:7;
     unsigned cyl:8;
     unsigned head:4;
     unsigned sect:7;
     unsigned num_of_sectors:5;
} DISK_REGISTER;

#endif

