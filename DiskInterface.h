//
// Created by kamil on 1/12/19.
//

#ifndef FILESYSTEM_DISKINTERFACE_H
#define FILESYSTEM_DISKINTERFACE_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "structs.h"

static const dataBlock emptyDataBlock;
static const iNode emptyNode;

/*
 * disk management functions
 */
FILE* openVirtualDisk(char *pathToVirtualDisk);
void createDisk(char *pathToDisk, uint32 nbytes);

/*
 * main interface to read/write from file
 */
uint32 writeData(FILE *file, void *buffer, uint32 bytesToWrite, uint32 offset);
uint32 readData(FILE *file, void *buffer, uint32 bytesToRead, uint32 offset);

/*
 * superBlock functions
 */
uint32 writeSuperBlock(FILE *disk, void* buffer);
uint32 readSuperBlock(FILE *disk, void* buffer);

/*
 * iNodes bitmap functions
 */
uint32 writeInodeToBitmap(FILE *disk, void* buffer, uint32 index);
uint32 readInodeFromBitmap(FILE *disk, void *buffer, uint32 index);
uint32 setInodeStatusToBitmap(FILE *disk, uint32 index, byte value);
uint32 writeInodesBitmap(FILE *disk, void* buffer);
uint32 readInodesBitmap(FILE *disk, void* buffer);

/*
 * iNodes functions
 */
uint32 writeInode(FILE *disk, void* buffer, uint32 index);
uint32 readInode(FILE *disk, void *buffer, uint32 index);

/*
 * dataBlocks bitmap functions
 */
uint32 writeDataBlockToBitmap(FILE *disk, void* buffer, uint32 index);
uint32 readDataBlockFromBitmap(FILE *disk, void *buffer, uint32 index);
uint32 setDataBlockStatusToBitmap(FILE *disk, uint32 index, byte value);
uint32 readDataBlocksBitmap(FILE *disk, void *buffer);

/*
 * write file to disk
 */
int32 writeFileToVirtualDisk(FILE *disk, char *fileToCopy);
int32 readFileFromVirtualDisk(FILE *disk, char *filename, char *pathToSave);

/*
 * deleting file from virtual disk
 */
int32 deleteFile(FILE *disk, char *fileToDelete);



/*
 *  some utility functions
 */
uint32 getFirstFreeInode(FILE *disk);
uint16 getFirstFreeDataBlock(FILE *disk);

uint32 isNameUnique(FILE *disk, char *filename);
uint32 isEnoughSpace(FILE *disk, uint32 length);
uint32 isEnoughInodes(FILE *disk);

uint32 findInode(FILE *disk, char *filename);

uint32 printInodesBitmap(FILE *disk);
uint32 printDataBitmap(FILE *disk);


/*
 *
 */
FILE* openFile(char *pathToFile);
uint32 getFileLength(FILE *file);

uint32 writeDataBlock(FILE *disk, dataBlock* data, uint32 blockIndex);
uint32 readDataBlock(FILE *disk, dataBlock* data, uint32 blockIndex);


/*
 * disk management functions
 */
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
		writeInode(disk, inode, i);

	for(uint32 i = 0; i<sBlock->maxDataBlocks; ++i)
		writeData(disk, &zeroByte, size(byte), dataBitmapOffset+i*size(byte));

	fclose(disk);
	free(sBlock);
	free(bitmapOfInodes);
	free(inode);
}

/*
 *                                  superBlock functions
 *
 */
uint32 writeSuperBlock(FILE *disk, void* buffer)
{
	return writeData(disk, buffer, size(superBlock), 0);
}

uint32 readSuperBlock(FILE *disk, void* buffer)
{
	return readData(disk, buffer, size(superBlock), 0);
}

/*
 *                                    iNodes bitmap functions
 *
 */
uint32 writeInodeToBitmap(FILE *disk, void* buffer, uint32 index)
{
	return writeData(disk, buffer, size(byte), iNodesBitmapOffset + index * size(byte));
}

uint32 readInodeFromBitmap(FILE *disk, void *buffer, uint32 index)
{
	return readData(disk, buffer, size(byte), iNodesBitmapOffset + index * size(byte));
}

uint32 setInodeStatusToBitmap(FILE *disk, uint32 index, byte value)
{
	byte givenValue = value;
	return writeInodeToBitmap(disk, &value, index);
}

uint32 writeInodesBitmap(FILE *disk, void* buffer)
{
	return writeData(disk, buffer, size(iNodesBitmap), iNodesBitmapOffset);
}

uint32 readInodesBitmap(FILE *disk, void* buffer)
{
	return readData(disk, buffer, size(iNodesBitmap), iNodesBitmapOffset);
}

/*
 *                                          iNodes functions
 *
 */
uint32 writeInode(FILE *disk, void* buffer, uint32 index)
{
	return writeData(disk, buffer, size(iNode), iNodesOffset + index * size(iNode));
}

uint32 readInode(FILE *disk, void *buffer, uint32 index)
{
	return readData(disk, buffer, size(iNode), iNodesOffset + index * size(iNode));
}

/*
 *                                          dataBlocks bitmap functions
 *
 */
uint32 writeDataBlockToBitmap(FILE *disk, void* buffer, uint32 index)
{
	return writeData(disk, buffer, size(byte), dataBitmapOffset + index*size(byte));
}

uint32 readDataBlockFromBitmap(FILE *disk, void *buffer, uint32 index)
{
	return readData(disk, buffer, size(byte), dataBitmapOffset + index*size(byte));
}

uint32 setDataBlockStatusToBitmap(FILE *disk, uint32 index, byte value)
{
	byte givenValue = value;
	return writeDataBlockToBitmap(disk, &value, index);
}

uint32 readDataBlocksBitmap(FILE *disk, void *buffer)
{
	superBlock sb;
	readSuperBlock(disk, &sb);
	return readData(disk, buffer, sb.maxDataBlocks*size(byte), dataBitmapOffset);
}

/*
 *                                      main interface to read/write from file
 */
uint32 writeData(FILE *file, void *buffer, uint32 bytesToWrite, uint32 offset)
{
	fseek(file, offset, SEEK_SET);
	return (uint32)fwrite(buffer, bytesToWrite, 1, file );
}

uint32 readData(FILE *file, void *buffer, uint32 bytesToRead, uint32 offset)
{
	fseek(file, offset, SEEK_SET);
	return (uint32)fread(buffer, bytesToRead, 1, file );
}

/*
 *
 */
uint32 getFirstFreeInode(FILE *disk)
{
	byte iNodeBit[MAX_FILES];
	readInodesBitmap(disk, iNodeBit);
	for (uint32 i = 0; i < MAX_FILES; ++i)
	{
		if(iNodeBit[i] == 0)
			return i;
	}
	return MAX_FILES;
}

uint16 getFirstFreeDataBlock(FILE *disk)
{
	superBlock sb;
	readSuperBlock(disk, &sb);
	byte *dataBlocksMap = malloc(sb.maxDataBlocks*size(byte));
	readDataBlocksBitmap(disk, dataBlocksMap);
	for (uint16 i = 0; i < sb.maxDataBlocks; ++i)
	{
		if(dataBlocksMap[i] == 0)
		{
			free(dataBlocksMap);
			return i;
		}
	}
	free(dataBlocksMap);
	return sb.maxDataBlocks;
}

uint32 isNameUnique(FILE *disk, char *filename)
{
	byte iNodesBitmap[MAX_FILES];
	iNode node = {0};
	readInodesBitmap(disk, iNodesBitmap);
	for(uint32 i = 0; i < MAX_FILES; ++i)
	{
		if(iNodesBitmap[i] == 0)
			continue;

		readInode(disk, &node, i);
		if( strcmp(node.name, filename) == 0)
			return 0;
	}
	return 1;
}

uint32 isEnoughSpace(FILE *disk, uint32 length)
{
	superBlock sb;
	readSuperBlock(disk, &sb);
	return sb.userSpace > length ? 1 : 0 ;
}

uint32 isEnoughInodes(FILE *disk)
{
	superBlock sb;
	readSuperBlock(disk, &sb);
	return  sb.usedInodes < MAX_FILES ? 1 : 0 ;
}

uint32 findInode(FILE *disk, char *filename)
{
	iNodesBitmap nodesBitmap = {0};
	iNode node = {0};
	readInodesBitmap(disk, &nodesBitmap);

	for (uint32 i = 0; i < MAX_FILES; ++i)
	{
		if(nodesBitmap.bitmap[i] == 1)
		{
			readInode(disk, &node, i);
			if(strcmp(filename, node.name) == 0)
				return i;
		}
	}
	return MAX_FILES;
}

uint32 printInodesBitmap(FILE *disk)
{
	iNodesBitmap bitmap ={0};
	readInodesBitmap(disk, &bitmap);
	printf("InodesBitmap:\n");
	for (uint32 i = 0; i < MAX_FILES; ++i)
	{
		printf("%d", bitmap.bitmap[i]);
	}
	printf("\n");
}

uint32 printDataBitmap(FILE *disk)
{
	superBlock sb;
	readSuperBlock(disk, &sb);

	byte *buffer = malloc(sb.maxDataBlocks * size(byte));
	readDataBlocksBitmap(disk, buffer);
	printf("DataBitmap:\n");
	for (uint32 i = 0; i < sb.maxDataBlocks; ++i)
	{
		printf("%d",buffer[i]);
	}
	printf("\n");
	free(buffer);
}

uint32 isFileNameProperLength(char *fileToCopy)
{
	return MAX_NAME_LENGTH > strlen(fileToCopy)+1 ? 1 : 0;
}

int writeFileToVirtualDisk(FILE *disk, char *fileToCopy)
{
	FILE *file;
	iNode node = {0};
	dataBlock data = {0};
	uint32 i = 0;

	superBlock sb;
	readSuperBlock(disk, &sb);

	if( !isEnoughInodes(disk))
		return -1;
	if( !isNameUnique(disk, fileToCopy))
		return -2;
	if( !isFileNameProperLength(fileToCopy))
		return -3;
	if( (file=openFile(fileToCopy))==NULL )
		return -4;

	uint32 fileLength = getFileLength(file);
	node.fileSize = fileLength;
	strcpy(node.name, fileToCopy);

	uint32 inodeIndex = getFirstFreeInode(disk);
	setInodeStatusToBitmap(disk, inodeIndex, 1);

	uint16 dataBlockIndex = getFirstFreeDataBlock(disk);
	setDataBlockStatusToBitmap(disk, dataBlockIndex, 1);

	node.firstBlockIndex = dataBlockIndex;
	writeInode(disk, &node, inodeIndex);

	while(fileLength > BLOCK_SIZE)
	{
		fileLength-=BLOCK_SIZE;
		uint16 newIndex = getFirstFreeDataBlock(disk);
		setDataBlockStatusToBitmap(disk, newIndex, 1);
		data.nextBlockIndex = newIndex;

		readData(file, &data.data, BLOCK_SIZE, i*BLOCK_SIZE);
		writeDataBlock(disk, &data, dataBlockIndex);

		++i;

		data = emptyDataBlock; //zero data struct
		dataBlockIndex = newIndex;

		if(fileLength < BLOCK_SIZE)
			break;
	}

	data.nextBlockIndex = sb.maxDataBlocks;
	readData(file, &data.data, fileLength, i*BLOCK_SIZE);
	writeDataBlock(disk, &data, dataBlockIndex);

	++(sb.usedInodes);
	sb.userSpace -= (i+1)*BLOCK_SIZE;
	sb.usedDataBlocks +=(i+1);


	fclose(file);
}

int32 deleteFile(FILE *disk, char *fileToDelete)
{
	if(isNameUnique(disk, fileToDelete) == 1)
		return -1;

	iNodesBitmap nodesBitmap={0};
	iNode node ={0};
	readInodesBitmap(disk, &nodesBitmap);
	superBlock sb;
	dataBlock data ={0};
	uint32 i=0;
	readSuperBlock(disk, &sb);

	for ( i = 0; i < MAX_FILES; ++i)
	{
		if(nodesBitmap.bitmap[i] == 1)
		{
			readInode(disk, &node, i);
			if(strcmp(node.name, fileToDelete) == 0)
			{
				setInodeStatusToBitmap(disk, i, 0);
				--(sb.usedInodes);
				break;
			}
		}
	}

	for (uint32 j = node.firstBlockIndex; j < sb.maxDataBlocks;)
	{
		readDataBlock(disk, &data, j);
		setDataBlockStatusToBitmap(disk, j, 0);
		--(sb.usedDataBlocks);
		sb.userSpace += BLOCK_SIZE;
		j = data.nextBlockIndex;
	}
	writeSuperBlock(disk, &sb);
	node = emptyNode;
	writeInode(disk, &node, i);
}

int32 readFileFromVirtualDisk(FILE *disk, char *filename, char *pathToSave)
{
	FILE *file;
	uint32 iNodeIndex, fileLength;
	iNode node;
	dataBlock data = {0};
	superBlock sb;
	readSuperBlock(disk, &sb);

	iNodeIndex = findInode(disk, filename);
	readInode(disk, &node, iNodeIndex);
	fileLength = node.fileSize;
	file = fopen(pathToSave, "w+b");
	truncate(pathToSave, fileLength);

	readDataBlock(disk, &data, node.firstBlockIndex);

	while( fileLength > BLOCK_SIZE )
	{
		uint32 nextIndex = data.nextBlockIndex;
		fileLength -= BLOCK_SIZE;
		fwrite(&data.data, BLOCK_SIZE, 1, file);

		data = emptyDataBlock;
		readDataBlock(disk, &data, nextIndex);
	}

	fwrite(&data.data, fileLength, 1, file);
	fclose(file);

	return 0;
}

FILE* openFile(char *pathToFile)
{
	return fopen(pathToFile, "rb");
}

uint32 getFileLength(FILE *file)
{
	fseek(file, 0, SEEK_END);
	uint32 position = (uint32)ftell(file);
	rewind(file);

	return position;
}

uint32 writeDataBlock(FILE *disk, dataBlock* data, uint32 blockIndex)
{
	superBlock sb;
	readSuperBlock(disk, &sb);

	return writeData(disk, data, size(dataBlock), sb.dataBlocksOffset + blockIndex*size(dataBlock));
}

uint32 readDataBlock(FILE *disk, dataBlock* data, uint32 blockIndex)
{
	superBlock sb;
	uint32 read;
	readSuperBlock(disk, &sb);

	return readData(disk, data, size(dataBlock), sb.dataBlocksOffset + blockIndex * size(dataBlock));
}
#endif //FILESYSTEM_DISKINTERFACE_H
