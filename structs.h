//
// Created by kamil on 1/12/19.
//

#ifndef FILESYSTEM_STRUCTS_H
#define FILESYSTEM_STRUCTS_H

#include <stdint.h>

#define BLOCK_SIZE 4096
#define MAX_NAME_LENGTH 32
#define MAX_FILES 100
#define size(b) ( ( sizeof(b) ) )


typedef uint32_t uint32;
typedef uint8_t int8;
typedef uint16_t uint16;
typedef char byte;


typedef struct dataBlock
{
	byte data[BLOCK_SIZE];
	uint32 nextBlockIndex;

} dataBlock;

typedef struct iNode
{
	uint16 usedBlocks;
	uint16 firstBlockIndex;
	uint32 fileSize;
	char name[MAX_NAME_LENGTH];

} iNode;

typedef struct superBlock
{
	uint16 maxDataBlocks;
	uint16 dataBlocksOffset;
	uint32 diskSize;
	uint32 userSpace;
	uint32 usedInodes;
	uint32 usedDataBlocks;
} superBlock;

typedef struct iNodesBitmap
{
	char bitmap[MAX_FILES];
} iNodesBitmap;

//
//typedef struct dataBlocksBitmap
//{
//	char bitmap[MAX_FILES];
//} dataBlocksBitmap;

#define iNodesBitmapOffset size(superBlock)
#define iNodesOffset (iNodesBitmapOffset + MAX_FILES)
#define dataBitmapOffset ( iNodesOffset + size(iNode)*MAX_FILES)
#define maxDataBlocks(nbytes) ( (nbytes-dataBitmapOffset)/size(dataBlock) )
#define bytesRequiresBySystem dataBitmapOffset

#define unsusedSpace(nbytes) ( nbytes - dataBitmapOffset )
#endif //FILESYSTEM_STRUCTS_H
