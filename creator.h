#ifndef __CREATOR_H
#define __CREATOR_H

#include "assembler.h"
#include "conversion.h"
#include "validation.h"
#include "parser.h"

dataStructure *initDataStructure(void);
symbolNode* newNode(char *symbolName, int symbolType, int symbolValue, int numOfLines/*, symbolNode *next*/);
int createObjectFile(char *fileName, dataStructure *memoryPtr);
int createEntryFile(char *fileName, dataStructure *memoryPtr);
int createExternalFile(char *fileName, dataStructure *memoryPtr);

#endif
