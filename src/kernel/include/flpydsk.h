#ifndef _FLPYDSK_DRIVER_H
#define _FLPYDSK_DRIVER_H

#include "os.h"

// install floppy driver
void flpydsk_install(int irq);

// set current working drive
void flpydsk_set_working_drive(unsigned char drive);

// get current working drive
unsigned char flpydsk_get_working_drive();

// read sector
unsigned char* flpydsk_read_sector(int sectorLBA);

// write sector
int flpydsk_write_sector(int sectorLBA);

// convert LBA to CHS
void flpydsk_lba_to_chs(int lba, int* head, int* track, int* sector);

#endif
