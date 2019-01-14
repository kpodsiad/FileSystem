//
// Created by kamil on 1/12/19.
//

#ifndef FILESYSTEM_DISKINTERFACE_H
#define FILESYSTEM_DISKINTERFACE_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "structs.h"

FILE* openVirtualDisk(char *pathToVirtualDisk);
void createDisk(char *pathToDisk, uint32 nbytes);

uint32 writeSuperBlock(FILE *disk, void* buffer);
uint32 readSuperBlock(FILE *disk, void* buffer);

uint32 writeInodeToBitmap(FILE *disk, void* buffer, uint32 index);
uint32 readInodeFromBitmap(FILE *disk, void *buffer, uint32 index);

uint32 writeInodesBitmap(FILE *disk, void* buffer);
uint32 readInodesBitmap(FILE *disk, void* buffer);

uint32 writeInode(FILE *disk, void* buffer, uint32 index);
uint32 readInode(FILE *disk, void *buffer, uint32 index);

uint32 writeData(FILE *file, void *buffer, uint32 bytesToWrite, uint32 offset);
uint32 readData(FILE *file, void *buffer, uint32 bytesToRead, uint32 offset);

uint32 readDataBlock(FILE *disk, void* buffer, uint32 blockIndex);

uint32 getFirstFreeInode(FILE *disk);

FILE* openFile(char *pathToFile);

FILE* openVirtualDisk(char *pathToVirtualDisk)
{
	FILE *disk;

	disk = fopen(pathToVirtualDisk, "r+b");

	if(disk == NULL)
		createDisk(pathToVirtualDisk, 1000000);
	else
		return disk;

	return fopen(pathToVirtualDisk, "r+b");
}

void createDisk(char *pathToDisk, uint32 bytes)
{
	FILE *disk;
	iNodesBitmap *bitmapOfInodes;
	iNode *inode;
	byte zeroByte = 0;
	superBlock *sBlock = (struct superBlock*)calloc(1, size(superBlock));
	disk = fopen(pathToDisk, "w+b");
	if(disk == NULL)
	{
		free(sBlock);
		perror("Unable to open virtual disk\n");
	}
	truncate(pathToDisk, bytes);

	sBlock->maxDataBlocks = maxDataBlocks(bytes);

	uint32 freeSpace = bytes - dataBitmapOffset - sBlock->maxDataBlocks*size(dataBlock);

	if( freeSpace >= sBlock->maxDataBlocks) //is there enough bytes for dataBlocksBitmap? 1 byte per 1 dataBlock is needed
		sBlock->dataBlocksOffset = dataBitmapOffset + sBlock->maxDataBlocks;
	else
		--(sBlock->maxDataBlocks); //decrement maximum number of dataBlocks

	sBlock->dataBlocksOffset = dataBitmapOffset + sBlock->maxDataBlocks;
	freeSpace = bytes - sBlock->dataBlocksOffset - sBlock->maxDataBlocks*size(dataBlock);

	truncate(pathToDisk, bytes-freeSpace);
	sBlock->diskSize = bytes-freeSpace;
	sBlock->userSpace = (sBlock->maxDataBlocks-sBlock->usedDataBlocks)*size(dataBlock);

	writeSuperBlock(disk, sBlock);
	bitmapOfInodes = calloc(1, size(iNodesBitmap));
	writeData(disk, bitmapOfInodes, size(iNodesBitmap), iNodesBitmapOffset);
	inode = calloc(1, size(iNode));

	for(uint32 i = 0; i<MAX_FILES; ++i)
		writeData(disk, inode, size(iNode), 0);

	for(uint32 i = 0; i<sBlock->maxDataBlocks; ++i)
		writeData(disk, &zeroByte, size(byte), 0);

	fclose(disk);
	free(sBlock);
	free(bitmapOfInodes);
	free(inode);
}

uint32 writeSuperBlock(FILE *disk, void* buffer)
{
	return writeData(disk, buffer, size(superBlock), 0);
}

uint32 readSuperBlock(FILE *disk, void* buffer)
{
	return readData(disk, buffer, size(superBlock), 0);
}

uint32 writeInodeToBitmap(FILE *disk, void* buffer, uint32 index)
{
	return writeData(disk, buffer, 1, iNodesBitmapOffset + index * size(byte));
}

uint32 readInodeFromBitmap(FILE *disk, void *buffer, uint32 index)
{
	return readData(disk, buffer, 1, iNodesBitmapOffset + index * size(byte));
}

uint32 writeInodesBitmap(FILE *disk, void* buffer)
{
	return writeData(disk, buffer, size(iNodesBitmap), iNodesBitmapOffset);
}

uint32 readInodesBitmap(FILE *disk, void* buffer)
{
	return readData(disk, buffer, size(iNodesBitmap), iNodesBitmapOffset);
}

uint32 writeInode(FILE *disk, void* buffer, uint32 index)
{
	return writeData(disk, buffer, size(iNode), iNodesOffset + index * size(iNode));
}

uint32 readInode(FILE *disk, void *buffer, uint32 index)
{
	return readData(disk, buffer, size(iNode), iNodesOffset + index * size(iNode));
}

uint32 writeData(FILE *file, void *buffer, uint32 bytesToWrite, uint32 offset)
{
	if(offset != 0)
		fseek(file, offset, SEEK_SET);

	return (uint32)fwrite(buffer, bytesToWrite, 1, file );
}

uint32 readData(FILE *file, void *buffer, uint32 bytesToRead, uint32 offset)
{
	fseek(file, offset, SEEK_SET);
	return (uint32)fread(buffer, bytesToRead, 1, file );
}

uint32 readDataBlock(FILE *disk, void* buffer, uint32 blockIndex)
{
	superBlock sb;
	uint32 read;
	readSuperBlock(disk, &sb);

	if(blockIndex >= sb.maxDataBlocks)
		perror("Invalid dataBlock index");

	read = readData(disk, buffer, size(dataBlock), sb.dataBlocksOffset + blockIndex * size(dataBlock));

	return read;
}

uint32 getFirstFreeInode(FILE *disk)
{
	superBlock sb;
	readSuperBlock(disk, &sb);
}

FILE* openFile(char *pathToFile)
{

}


//uint32 readBlock(char *filename, uint32 blockNumber, void *block);
//uint32 writeBlock(char *filename, uint32 blockNumber, void *block);
//void syncDisk();
//
//FILE* openDisk(char *pathToDisk);
//void createDisk(char *pathToDisk, uint32 nbytes);
//
//uint32 writeData(char *pathToDisk, uint32 offset, uint32 bytesToWrite, void *buffer);
//uint32 readData(char *pathToDisk, uint32 offset, uint32 bytesToWrite, void *buffer);
//
//uint32 copyFileFromHardDisk(char *pathToDisk, char *pathToFile);
//
//uint32 getFileLength(FILE *file);
//uint32 getFirstFreeInode(FILE *disk);
//uint32 getFirstFreeDataBlock(FILE *disk);
//
//void initializeValues(char *pathToDisk, superBlock *sBlock);
//void initializeSuperBlock(char *pathToDisk, uint32 bytes, superBlock *sBlock);
//
//FILE* openDisk(char *pathToDisk)
//{
//	FILE *disk;
//
//	disk = fopen(pathToDisk, "r+b");
//
//	if(disk == NULL)
//		createDisk(pathToDisk, 1000000);
//	else
//		return disk;
//
//	return fopen(pathToDisk, "r+b");
//}
//
//void createDisk(char *pathToDisk, uint32 bytes)
//{
//	FILE *disk;
//	superBlock *sBlock = (struct superBlock*)malloc(size(superBlock));
//
//
//	disk = fopen(pathToDisk, "w+b");
//	if(disk == NULL)
//	{
//		perror("Unable to open virtual disk\n");
//	}
//	truncate(pathToDisk, bytes);
//	fclose(disk);
//
//	initializeSuperBlock(pathToDisk, bytes, sBlock);
//	initializeValues(pathToDisk, sBlock);
//
//	free(sBlock);
//}
//
//uint32 copyFileFromHardDisk(char *pathToDisk, char *pathToFile )
//{
//	FILE *file, *disk;
//	superBlock sb;
//	iNode iNode;
//	byte *array;
//
//	file = fopen(pathToFile, "rb");
//	if(file == NULL)
//		perror("Unable to open file from hard disk\n");
//
//	disk = fopen(pathToDisk, "r+b");
//	if(disk == NULL)
//		perror("Unable to open virtual disk\n");
//
//	fread(&sb, 1, size(superBlock), disk);
//
//	uint32 fileLength = getFileLength(file);
//	if(fileLength > sb.userSpace)
//		perror("There is not enough space\n");
//
//	uint32 iNodeIndex = getFirstFreeInode(disk);
//	if(iNodeIndex == MAX_FILES)
//		perror("There is no free iNode\n");
//
//	uint32 fileNameLength = (uint32)strlen(pathToFile);
//	if(fileNameLength > MAX_NAME_LENGTH)
//		perror("The name of file is too long\n");
//
//	uint32 blockIndex = getFirstFreeDataBlock(disk);
//	if(blockIndex == sb.maxDataBlocks)
//		perror("There is not enough free blocks\n");
//
//	iNode.fileSize = fileLength;
//	strcpy(iNode.name, pathToFile);
//	iNode.firstBlockIndex = (uint16)blockIndex;
//
//	if(fileLength < BLOCK_SIZE)
//	{
//		iNode.usedBlocks = 1;
//		array = malloc(fileLength);
//		fread(array, 1, fileLength, file);
//		fseek(disk, 0, sb.dataBlocksOffset+blockIndex*BLOCK_SIZE);
//		fwrite(array, 1, fileLength, disk);
//		free(array);
//	}
//	else
//	{
//
//	}
//
//	fwrite(&sb, 1, size(superBlock), disk);
//	fwrite(&iNode, 1, iNodesOffset+size(iNode)*iNodeIndex, disk);
//
//	fclose(disk);
//	fclose(file);
//}
//
//uint32 getFirstFreeInode(FILE *disk)
//{
//	byte iNode = 1;
//	uint32 read = 0;
//	fseek(disk, iNodesBitmapOffset, 0);
//	for(uint32 i=0; i<MAX_FILES; ++i)
//	{
//		read = (uint32)fread(&iNode, 1, 1, disk);
//
//		if(iNode == 0)
//			return i;
//	}
//
//	return MAX_FILES;
//}
//
//uint32 getFirstFreeDataBlock(FILE *disk)
//{
//	byte dataBlock = 1;
//	superBlock sb;
//	fread(&sb, 1, size(superBlock), disk);
//	fseek(disk, sb.dataBlocksOffset, 0);
//	for(uint32 i=0; i<sb.maxDataBlocks; ++i)
//	{
//		fread(&dataBlock, 1, 1, disk);
//		if(dataBlock == 0)
//			return i;
//	}
//
//	return sb.maxDataBlocks;
//}
//
//
//uint32 getFileLength(FILE *file)
//{
//	uint32 length;
//	fseek(file, 0, SEEK_END);
//	length = (uint32)ftell(file);
//	fseek(file, 0, 0);
//	return length;
//}
//
//void initializeSuperBlock(char *pathToDisk, uint32 bytes, superBlock *sBlock)
//{
//	sBlock->diskSize = bytes;
//	sBlock->usedDataBlocks = 0;
//	sBlock->usedInodes = 0;
//	//uint32 dBOffset = dataBitmapOffset;
//	sBlock->maxDataBlocks = maxDataBlocks(bytes);
//
//	uint32 freeSpace = sBlock->diskSize - dataBitmapOffset - sBlock->maxDataBlocks*size(dataBlock);
//
//	if( freeSpace > sBlock->maxDataBlocks) //is there enough bytes for dataBlocksBitmap? 1 byte per 1 dataBlock is needed
//	{
//		sBlock->dataBlocksOffset = dataBitmapOffset + sBlock->maxDataBlocks;
//	}
//	else
//	{
//		--(sBlock->maxDataBlocks); //decrement maximum number of dataBlocks
//		sBlock->dataBlocksOffset = dataBitmapOffset + sBlock->maxDataBlocks;
//	}
//	//freeSpace = sBlock->diskSize - dataBitmapOffset - sBlock->maxDataBlocks*size(dataBlock);
//
//	sBlock->userSpace = (sBlock->maxDataBlocks-sBlock->usedDataBlocks)*size(dataBlock);
//	//uint32 unusedSpace = bytes - sBlock->dataBlocksOffset - sBlock->maxDataBlocks*size(dataBlock);
//	writeData(pathToDisk, 0, 0, sBlock);
//
//}
//
//void initializeValues(char *pathToDisk, superBlock *sBlock)
//{
//	iNodesBitmap iNodeBM = {0};
//	iNode iNode = {0};
//	byte zeroByte = 0;
//
//	writeData(pathToDisk, iNodesBitmapOffset, size(iNodesBitmap), &iNodeBM);
//	for (int i = 0; i < MAX_FILES ; ++i)
//	{
//		uint32 offset = iNodesOffset + i*size(iNode);
//		writeData(pathToDisk, iNodesOffset + i*size(iNode), size(iNodesBitmap), &iNodeBM);
//	}
//	writeData(pathToDisk, dataBitmapOffset, sBlock->maxDataBlocks, &zeroByte);
//}
//
//uint32 writeData(char *pathToDisk, uint32 offset, uint32 bytesToWrite, void* buffer)
//{
//	FILE *disk;
//	uint32 written;
//
//	disk = openDisk(pathToDisk);
//
//	fseek(disk, offset, 0);
//	written = (uint32) fwrite(buffer, 1, bytesToWrite, disk);
//	fclose(disk);
//
//	return written;
//}
//
//uint32 readData(char *pathToDisk, uint32 offset, uint32 bytesToRead, void* buffer)
//{
//	FILE *disk;
//	uint32 read = 0;
//
//	disk = openDisk(pathToDisk);
//
//	fseek(disk, offset, 0);
//	read = (uint32)fread(buffer, 1, bytesToRead, disk);
//	fclose(disk);
//
//	return read;
//}
//
//uint32 readBlock(char *filename, uint32 blockNumber, void *block)
//{
//	FILE *disk;
//	uint32 read = 0;
//	superBlock sb;
//
//	readData(filename, 0, size(superBlock), &sb);
//	disk = openDisk(path);
//
//	fseek(disk, sb.dataBlocksOffset+blockNumber*BLOCK_SIZE, 0);
//	read = (uint32)fread(block, sizeof(uint32), 1, disk);
//	fclose(disk);
//
//	return read;
//}
//
//uint32 writeBlock(char *filename, uint32 blockNumber, void *block)
//{
//	FILE *disk;
//	uint32 written;
//	superBlock sb;
//
//	readData(filename, 0, size(superBlock), &sb);
//	disk = openDisk(path);
//
//	fseek(disk, sb.dataBlocksOffset + blockNumber*BLOCK_SIZE, 0);
//	written = (uint32) fwrite(block, sizeof(uint32), 1, disk);
//	++sb.usedDataBlocks;
//	writeData(filename, 0, size(superBlock), &sb);
//	fclose(disk);
//
//	return written;
//}


#endif //FILESYSTEM_DISKINTERFACE_H
