#ifndef __ASSEMBLER_H
#define __ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>

#define MAXLINE 81
#define MAXMEM 4096
#define MAXWORD 15
#define MAXLABEL 32
#define NUMILLEGALNAMES 34
#define MAXDECIMALBOUNDS 8191
#define MINDECIMALBOUNDS -8192
#define MAX12BITDECIMALBOUNDS 2047
#define MIN12BITDECIMALBOUNDS -2048
#define MAXPATH 2048

extern char *illegalNames[];

enum operation
{
	mov = 0,
	cmp = 1,
	add = 2,
	sub = 3,
	not = 4,
	clr = 5,
	lea = 6,
	inc = 7,
	dec = 8,
	jmp = 9,
	bne = 10,
	red = 11,
	prn = 12,
	jsr = 13,
	rts = 14,
	stop = 15
};

enum symbolType
{
	macro = 0,
	code = 1,
	data = 2,
	external = 3,
	entry = 4,
	symbolInLine = 5
};

enum addressingMethod
{
	immediate = 0,
	direct = 1,
	permanentIndex = 2,
	directRegister = 3
};

/*Status of the word in the memory array */
enum wordStatus
{
	empty = 0,
	convertedToBinary = 1,
	labelNeedsToBeChecked = 2 /* 2 - label name that needs to be checked if exists yet or not */
};

typedef struct symbolNode
{
	char symbolName[MAXLINE];
	int symbolType; /* symbolInLine = 5, if symbol encountered in code and needs to be identified as entry or external */
	int symbolValue; /*memory address in memory array. -1 if needs to be fixed during second pass */
	int numOfLines; /*Holds the number of lines the data has. Relevant only for data and string types */
	struct symbolNode* next;
} symbolNode;

typedef struct errorListNode
{
	int lineNumInFile;
	int errorCode;
	struct errorListNode* next;
} errorListNode;

typedef struct dataStructure
{
	struct errorListNode *errorListHead;
	struct symbolNode *symbolListHead;
	char memoryArr[MAXMEM][MAXWORD];
	int IC;
	int DC;
} dataStructure;

typedef struct parserReport
{
	/*values for firstValue and secondValue,
	 * 0 - empty,
	 * 1 - converted to the correct binary value, just copy to memory array
	 * 2 - label name that needs to be checked if exists yet or not
	 */
	char values[2][MAXLABEL];
	int numValues;
	int firstValue;
	int secondValue;
	int addressingMethod;
	int registerNum;
} parserReport;

enum error_code
{
	illegal_macro_label = -101,
	illegal_macro_definition = -102,
	illegal_macro_value = -103,
	illegal_label = -104,
	illegal_label_before_macro = -105,
	illegal_string_definition = -106,
	memory_allocation_error = -107,
	label_too_long = -108,
	illegal_number_definition = -109,
	missing_comma = -110,
	extra_comma = -111,
	comma_before_first_data_input = -112,
	illegal_integer_sytnax = -113,
	double_comma = -114,
	text_after_legal_input = -115,
	number_out_of_bounds = -116,
	bad_addressing_method = -117,
	immediate_addressing_does_not_use_number_or_macro = -118,
	illegal_text_after_register = -119,
	wrong_addressing_method_for_command = -120,
	can_not_add_to_memory_will_overflow = -121,
	can_not_use_macro_in_direct_addressing = -122,
	duplicate_macro_definition = -123,
	entry_refers_to_illegal_label_name = -124,
	duplicate_entry_definition = -125,
	file_path_does_not_exist = -126,
	extraneous_text_after_command = -127,
	label_does_not_refer_to_macro = -128,
	label_has_not_been_defined_yet = -129,
	no_object_file_created = -130,
	no_entry_file_created = -131,
	no_external_file_created = -132,
	double_label_definition = -133,
	unknown_command = -134,

	empty_line = 101

};

int printErrorList(dataStructure *memoryPtr, char *fileName);
int addToErrorlist(int codeLine, int errorCode, dataStructure *memoryPtr);
int printSymbolList(symbolNode *head);
int printInstructionsArray(dataStructure *memoryPtr);
int getLabelData(char *labelToFind, int *labelType, int *labelAddress, dataStructure *memoryPtr);
int getSymbolData(char *symbolName, dataStructure *memoryPtr, int *memoryIndex, int *symbolType);
int secondPass(dataStructure *memoryPtr);
int fileReaderAndParser(FILE * pFile, dataStructure *memoryPtr);
int cleanUp(dataStructure *memoryPtr);
int main(int argc, char *argv[]);

#endif


