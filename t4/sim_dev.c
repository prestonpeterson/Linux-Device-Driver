/**
* Name: Preston Peterson
* Lab/task: Project 2 Task 4
* Date: 12/08/16
**/

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include "sim_dev.h"

// arbitrary number
#define SIM_DEV_MAJOR 567
#define SIM_DEV_NAME "sim_dev"

// this space holds the data that device users send to the device
static char *storage = NULL;
disk_t disk;
#define STORAGE_SIZE 4096

static unsigned long ioctl_control_data;
static unsigned long ioctl_status_data = 0x00000001;
static DISK_REGISTER *disk_status_reg = (DISK_REGISTER *) &ioctl_status_data;
static DISK_REGISTER *disk_control_reg = (DISK_REGISTER *) &ioctl_control_data;

static int validate_address(DISK_REGISTER *reg) {
	if (!reg->ready) {
		printk("SIM_DEV: register not ready");
		reg->error_code = ERDY;
		return ERDY;
	}
    if (reg->cyl >= NUM_OF_CYLS) {
		printk("SIM_DEV: Invalid cyl\n");
		reg->error_code = ECYL;
       	return ECYL;
	}
	if (reg->head >= NUM_OF_HEADS) {
		printk("SIM_DEV: Invalid head\n");
		reg->error_code = EHEAD;
       	return EHEAD;
	}
	if (reg->sect >= NUM_OF_SECTS) {
		printk("SIM_DEV: Invalid sect\n");
		reg->error_code = EHEAD;
       	return EHEAD;
	}
	if (reg->num_of_sectors == 0 || reg->num_of_sectors >= NUM_OF_SECTS) {
		reg->error_code = ESECTCOUNT;
		printk("SIM_DEV: Invalid num_of_sectors\n");
		return ESECTCOUNT;
	}
	return 0;
}

// open function - called when the "file" /dev/sim_dev is opened in userspace
static int sim_dev_open (struct inode *inode, struct file *file)
{
   // this is a special print functions that allows a user to print from the kernel
	printk("SIM_DEV_OPEN\n");
	return 0;
}

// close function - called when the "file" /dev/sim_dev is closed in userspace  
static int sim_dev_release (struct inode *inode, struct file *file)
{
	printk("SIM_DEV_RELEASE\n");
	return 0;
}

// read function called when  /dev/sim_dev is read
static ssize_t sim_dev_read( struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	int valid_address = validate_address(disk_control_reg); // error checking
	int sector = 0;
	int buffer_index = 0;
	int sector_index = 0;
	int sect = disk_control_reg->sect;
	printk("SIM_DEV_READ: cyl = %d. head = %d. sect = %d. num_of_sectors = %d\n", 
		disk_control_reg->cyl, disk_control_reg->head, disk_control_reg->sect, disk_control_reg->num_of_sectors);
		
	if (valid_address != 0)
		return valid_address;

	storage = kmalloc(SECT_SIZE * disk_control_reg->num_of_sectors, GFP_KERNEL);

	while (sector < disk_control_reg->num_of_sectors) {
		for (sector_index = 0; sector_index < SECT_SIZE; sector_index++) {
			if (disk[disk_control_reg->cyl][disk_control_reg->head][sect][sector_index] == '\0') {
				break;
			}
			storage[buffer_index] = disk[disk_control_reg->cyl][disk_control_reg->head][sect][sector_index];
			buffer_index++;
		}
		sector++;
		sect = (sect + 1) % NUM_OF_SECTS;
	}
	storage[buffer_index] = '\0';

	printk("READ \"%s\"\n", storage);

	if(copy_to_user(buf, storage, count) != 0) {
		disk_status_reg->error_code = -EFAULT;
		kfree(storage);
    	return -EFAULT;
	}
	kfree(storage);

	disk_status_reg->ready = 1;
	disk_status_reg->error_code = 0;
    return 0;
}

// write function called when /dev/sim_dev is written to
static ssize_t sim_dev_write( struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	int valid_address = validate_address(disk_control_reg);	
	int sector_index = 0;
	int buffer_index = 0;
	int i = disk_control_reg->sect;
	int sector = 0;
	disk_status_reg->ready = 0; // lock the device from reading during write operation
	printk("SIM_DEV_WRITE: cyl = %d. head = %d. sect = %d. num_of_sectors = %d\n", 
		disk_control_reg->cyl, disk_control_reg->head, disk_control_reg->sect, disk_control_reg->num_of_sectors);
	
	if (valid_address != 0)
		return valid_address;

	

	while (sector < disk_control_reg->num_of_sectors) {
		for (sector_index = 0; sector_index < SECT_SIZE; sector_index++) {
			disk[disk_control_reg->cyl][disk_control_reg->head][i][sector_index] = ((char *)buf)[buffer_index];
			if (((char *)buf)[buffer_index] == '\0') {
				buffer_index++;
				break;
			}
			buffer_index++;
		}
		sector++;
		i = (i + 1) % NUM_OF_SECTS;
	}
	printk("WRITING \"%s\"\n", disk[disk_control_reg->cyl][disk_control_reg->head][disk_control_reg->sect]);

	disk_control_reg->ready = 1;
	disk_control_reg->error_code = 0;
	msleep(3000);
	disk_status_reg->ready = 1; // allow device to be read from again
	return 0;
}


// ioctl function called when /dev/sim_dev receives an ioctl command
// Ubuntu 10.10: static int sim_dev_ioctl(struct inode *inode, struct file *file, unsigned int command, unsigned long arg)
// Ubuntu 11.04:
static long sim_dev_unlocked_ioctl(struct file *file, unsigned int command, unsigned long arg)
{
	int *p;
	switch ( command )
	{
		case IOCTL_SIM_DEV_WRITE:/* for writing data to arg */
			p = (int *)arg;
			printk("IOTCTL: Setting control reg to %d\n", *p);
			if (copy_from_user(&ioctl_control_data, (int *)arg, sizeof(int)))
			   return -EFAULT;
			break;
			
		case IOCTL_SIM_DEV_READ:/* for reading data from arg */
			printk("IOCTL: Device status register = %ld", ioctl_status_data);
			if (copy_to_user((int *)arg, &ioctl_status_data, sizeof(int)))
			   return -EFAULT;
			break;
			
		default:
			return -EINVAL;
	}
	return -EINVAL;
}

//
// mapping of file operations to the driver functions
//
struct file_operations sim_dev_file_operations = {
	.owner	=	THIS_MODULE,
	.llseek	=	NULL,
	.read		=	sim_dev_read,
	.write	=	sim_dev_write,
//	.readdir	=	NULL, // Ubuntu 14.04
	.poll		=	NULL,
//	.ioctl	=	sim_dev_ioctl, // Ubuntu 10.10
	.unlocked_ioctl	=	sim_dev_unlocked_ioctl, // Ubuntu 11.04
	.mmap		=	NULL,
	.open		=	sim_dev_open,
	.flush	=	NULL,
	.release	=	sim_dev_release,
	.fsync	=	NULL,
	.fasync	=	NULL,
	.lock		=	NULL,
};

// Loads a module in the kernel
static int sim_dev_init_module (void)
{
   // here we register sim_dev as a character device
	if (register_chrdev(SIM_DEV_MAJOR, SIM_DEV_NAME, &sim_dev_file_operations) != 0)
	   return -EIO;

	// reserve memory with kmalloc - Allocating Memory in the Kernel
	// GFP_KERNEL --> this does not have to be atomic, so kernel can sleep
	storage = kmalloc(STORAGE_SIZE, GFP_KERNEL);
	if (!storage) {
		printk("kmalloc failed\n");
		return -1;
	}
   printk("Simulated Driver Module Installed\n");
   return 0;
}

//  Removes module from kernel
static void sim_dev_cleanup_module(void)
{
   // specialized function to free memeory in kernel
	kfree(storage);
	unregister_chrdev (SIM_DEV_MAJOR, SIM_DEV_NAME);
   printk("Simulated Driver Module Uninstalled\n");
}

// map the module initialization and cleanup functins
module_init(sim_dev_init_module);
module_exit(sim_dev_cleanup_module);

MODULE_AUTHOR("http://www.cs.csuci.edu/~ajbieszczad");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simulated Device Linux Device Driver");

