#ifndef __VALIDATION_H
#define __VALIDATION_H

#include "assembler.h"
#include "creator.h"
#include "conversion.h"
#include "parser.h"

int checkDirectOrIndexAddressing(char *str, char **endPtr, parserReport *report, dataStructure *memoryPtr);
FILE * checkIfCodeFileExists(char *fileName);
int checkImmediateAddressing(char *str, char **endPtr, parserReport *report, dataStructure *memoryPtr);
int checkLegalDecimal(char *str);
int checkLegalLabel(char *str);
int checkRegisterAddressing(char *str, char **endPtr, parserReport *report, dataStructure *memoryPtr);
int checkIfLabelExistsInDataTable(char *label, int *symbolType, int *symbolValue, dataStructure *memoryPtr);
int checkIfLineIsDataOrString(int lineNum, dataStructure *memoryPtr);


#endif
