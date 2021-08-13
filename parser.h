#ifndef __PARSER_H
#define __PARSER_H

#include "assembler.h"
#include "creator.h"
#include "conversion.h"
#include "validation.h"

int dataParser(char *str, char **endPtr, char **labelPtrPtr, dataStructure *memoryPtr);
int defineParser(char *str, char **endPtr, char **symbolName, int* symbolValue, dataStructure *memoryPtr);
int entryParser(char *str, char **endPtr, char **labelPtrPtr, dataStructure *memoryPtr);
int externParser(char *str, char **endPtr, char **labelPtrPtr, dataStructure *memoryPtr);
int extractLabel(char *str, char **endPtr, char **label);
int extractSymbol(char *str, char **endPtr, char **label);
int rtsStopParser(char *str, char **endPtr, dataStructure *memoryPtr, int commandCode,
		char *label, int labelStatus);
int prnNotClrIncDecRedParser(char *str, char **endPtr, dataStructure *memoryPtr, int commandCode,
		char *label, int labelStatus, parserReport *reportFirstOperand);
int stringParser(char *str, char **endPtr, char **labelPtrPtr, dataStructure *memoryPtr);
int operandParser(char *str, char **endPtr, parserReport *report, dataStructure *memoryPtr);
int movCmpAddSubLeaParser(char *str, char **endPtr, dataStructure *memoryPtr, int commandCode,
		char *label, int labelStatus,  parserReport *reportFirstOperand,  parserReport *reportSecondOperand);
int parser(char *str, dataStructure *memoryPtr);
int skip_spaces(char *str, char **endptr);
int UpdateRegisterAddress(char *secondWord, int registerNum, int firstOrSecondRegister);

#endif
