#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "DiskInterface.h"
#include "structs.h"


int main()
{
	char *filename = "file";
	createDisk(filename, 1000000);


	FILE *disk = openVirtualDisk(filename);
	fclose(disk);
	return 0;
}
