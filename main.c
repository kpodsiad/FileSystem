#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "DiskInterface.h"
#include "structs.h"

void testInodeBitmapReadWrite(FILE *disk);
void testInodeReadWrite(FILE *disk);

int main()
{
	char *filename = "file";
	createDisk(filename, 1000000 + 662);
	superBlock sb;

	FILE *disk = openVirtualDisk(filename);
	readSuperBlock(disk, &sb);
	testInodeBitmapReadWrite(disk);
	testInodeReadWrite(disk);

	printf("%ld\n", size(superBlock));
//	printf("%ld\n", size(iNode));
//	printf("%ld\n", size(dataBlock));
//
//	printf("%d\n", sb.maxDataBlocks);
//
//	printf("%ld\n", dataBitmapOffset+sb.maxDataBlocks);
//	printf("%d\n", sb.dataBlocksOffset);

	fclose(disk);
	return 0;
}

void testInodeBitmapReadWrite(FILE *disk)
{
	byte bitmap[100];
	byte nonZeroByte = 99;
	//writeData(disk, iNodesBitmapOffset, size(byte), &nonZeroByte);
	writeInodeToBitmap(disk, &nonZeroByte, 1);
	//writeData(disk, iNodesBitmapOffset+5, size(byte), &nonZeroByte);
	//writeData(disk, iNodesBitmapOffset+15, size(byte), &nonZeroByte);
	//readData(disk, iNodesBitmapOffset, size(iNodesBitmap), bitmap);
	readInodeFromBitmap(disk, bitmap + 1, 1);
	printf("");
}

void testInodeReadWrite(FILE *disk)
{
	iNode node, nodeArray[100];
	strcpy(node.name, "xD");
	node.firstBlockIndex = 13;
	node.fileSize = 997;
	node.usedBlocks = 321;

	writeInode(disk, &node, 0);
	writeInode(disk, &node, 5);
	for (int i = 0; i < 100; ++i)
	{
		readInode(disk, nodeArray+i, i);
	}
	printf("");
}