#include "assembler.h"
#include "creator.h"
#include "conversion.h"
#include "validation.h"
#include "parser.h"


char *illegalNames[100] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
					"mov", "cmp", "add", "sub", "not", "clr", "lea",
					"inc", "dec", "jmp", "bne", "red", "prn", "jsr",
					"rts", "stop", "data", ".data", "string", ".string",
					"entry", ".entry", "extern", ".extern", "define", ".define"};

char errorCodesTexts[100][100] =
{"", /*Cell 0*/
"illegal macro label", /*Cell 1*/
"illegal macro definition",
"illegal macro value",
"illegal label",
"illegal label before macro",
"illegal string definition",
"memory allocation error",
"label too long",
"illegal number definition",
"missing comma", /*Cell 10*/
"extra comma",
"comma before first data input",
"illegal integer syntax",
"double comma",
"text after legal input",
"number out of bounds",
"bad addressing method",
"immediate addressing does not use number or macro",
"illegal text after register",
"wrong addressing method for command", /*Cell 20*/
"can not add to memory will overflow",
"can not use macro in direct addressing",
"duplicate macro definition",
"entry refers to illegal label name",
"duplicate entry definition",
"file path does not exist",
"extraneous text after command",
"label does not refer to macro",
"label has not been defined yet",
"no object file created", /*Cell 30*/
"no entry file created",
"no external file created",
"double label definition",
"unknown command",
};



int main(int argc, char *argv[])
{

	int i;

	dataStructure *memoryPtr = NULL;

	FILE * pFile = NULL;

	for (i = 1 ; i< argc; i++)
	{
		memoryPtr = initDataStructure();
		pFile = checkIfCodeFileExists(argv[i]);
		if (pFile!=NULL)
		{
			fileReaderAndParser(pFile, memoryPtr);
			fclose(pFile);
			secondPass(memoryPtr);

			if (memoryPtr->errorListHead==NULL)
			{
				createObjectFile(argv[i], memoryPtr);
				createEntryFile(argv[i], memoryPtr);
				createExternalFile(argv[i], memoryPtr);
			}
			else
			 printErrorList(memoryPtr, argv[i]);
			 cleanUp(memoryPtr);
		}
		else
			printf("File %s.as doesn't exist\n", argv[i]);
	}

	return 0;
}


/*Prints the error list to screen. memoryPtr is the main memory structure. fileName is the name of the
 * current assembly file.
 */
int printErrorList(dataStructure *memoryPtr, char *fileName)
{
	errorListNode *temp=NULL;

	temp = memoryPtr->errorListHead;

	if (temp!=NULL)
		printf("The following errors were found in file: %s\n", fileName);

	while (temp!=NULL)
	{
		printf("%s in line no. %d.\n", errorCodesTexts[(temp->errorCode)*(-1)-100], temp->lineNumInFile);
		temp = temp->next;
	}
	return 0;
}

/*
 * Adds a new node to the list of errors encountered during parsing of the assembly file.
 * codeLine - current line in assembly file
 * errorCode = error encountered
 */
int addToErrorlist(int codeLine, int errorCode, dataStructure *memoryPtr)
{
	errorListNode *temp=NULL, *loopTemp=NULL;

	temp = (errorListNode *)malloc(sizeof(errorListNode));
	if (temp==NULL)
		return memory_allocation_error;
	temp->errorCode = errorCode;
	temp->lineNumInFile = codeLine;
	temp->next = NULL;
	if (memoryPtr->errorListHead==NULL)
	{
		memoryPtr->errorListHead = temp;
		return 0;
	}

	loopTemp = memoryPtr->errorListHead;
	while (loopTemp->next!=NULL)
		loopTemp = loopTemp->next;
	loopTemp->next = temp;

	return 0;
}

/* function receives pointer to a list and prints the values of the list
 * (Was used for debugging).
 * */
int printSymbolList(symbolNode *head)
{
	symbolNode* tmp;

	tmp=head;

	while (tmp!=NULL)
	{
		printf("Symbol name is: %s, symbol type is: %d, symbol value is: %d, number of lines is, %d\n", (tmp->symbolName), tmp->symbolType, tmp->symbolValue, tmp->numOfLines);
		tmp=tmp->next;
	}

	return 0;
}

/* Function prints the memory array of the parsed assembly file
 * (Was used for debugging).
 */
int printInstructionsArray(dataStructure *memoryPtr)
{
	int i;

	for (i = 100; i < memoryPtr->IC ; i++)
	{
		printf("Memory address is:_%d, Binary Code:_%s\n", i, memoryPtr->memoryArr[i]);
	}

	return 0;
}

/*find if label exists in the data table and returns its properties
 * returns -1 if not found
 */
int getLabelData(char *labelToFind, int *labelType, int *labelAddress, dataStructure *memoryPtr)
{
	symbolNode *temp = NULL;

	temp = memoryPtr->symbolListHead;

	while (temp!=NULL)
	{
		if (strcmp(temp->symbolName, labelToFind)==0)
		{
			*labelType = temp->symbolType;
			*labelAddress = temp->symbolValue;
			return 0;
		}
		temp = temp->next;
	}
	return -1;
}

/*gets a symbol (label) name and returns it's properties from the symbols list */
int getSymbolData(char *symbolName, dataStructure *memoryPtr, int *memoryIndex, int *symbolType)
{
	symbolNode  *tempSymbolPtr=NULL;
	int memoryIndexFlag = -1, symbolTypeFlag = -1;

	tempSymbolPtr = memoryPtr->symbolListHead;

	while (tempSymbolPtr!=NULL )
	{
		if (strcmp(symbolName, tempSymbolPtr->symbolName)==0)
		{
			if (memoryIndexFlag == -1)
			{
				if (tempSymbolPtr->symbolType == code || tempSymbolPtr->symbolType == data)
				{
					*memoryIndex = tempSymbolPtr->symbolValue;
					memoryIndexFlag=1;
				}
			}
			if (symbolTypeFlag == -1)
			{
				if (tempSymbolPtr->symbolType == external)
				{
					*symbolType = tempSymbolPtr->symbolType;
					symbolTypeFlag = 1;
				}
			}
		}
		tempSymbolPtr = tempSymbolPtr->next;
	}
	return 0;
}

/*Function performs the "second pass" on the memory array and completes the missing
 * words bases on the symbol list.
 */
int secondPass(dataStructure *memoryPtr)
{
	symbolNode  *tempSymbolPtr=NULL;
	int memoryIndex = -1 , symbolType = -1, i;
	char twosCompliment[MAXWORD], *twosComplimentPtr = NULL, **twosComplimentPtrPtr = NULL;

	tempSymbolPtr = memoryPtr->symbolListHead;

	while (tempSymbolPtr!=NULL)
	{
		memoryIndex = -1;
		symbolType = -1;
		if (tempSymbolPtr->symbolType==symbolInLine)
		{
			getSymbolData(tempSymbolPtr->symbolName, memoryPtr, &memoryIndex, &symbolType);
			twosComplimentPtr = &twosCompliment[0];
			twosComplimentPtrPtr = &twosComplimentPtr;
			convertDecimalToTwosCompliment(memoryIndex, twosComplimentPtrPtr);
			for (i = 1; i< MAXWORD -3; i++)
				twosCompliment[i] = twosCompliment[i+2];
			strcpy(memoryPtr->memoryArr[tempSymbolPtr->symbolValue], twosCompliment);
			memoryPtr->memoryArr[tempSymbolPtr->symbolValue][12] = '1';
			memoryPtr->memoryArr[tempSymbolPtr->symbolValue][13] = '0';
			if (symbolType == external)
			{
				strcpy(memoryPtr->memoryArr[tempSymbolPtr->symbolValue], "00000000000001\0");
			}
			if (symbolType == entry)
			{
				memoryPtr->memoryArr[tempSymbolPtr->symbolValue][12] = '1';
				memoryPtr->memoryArr[tempSymbolPtr->symbolValue][13] = '0';
			}

		}
		tempSymbolPtr = tempSymbolPtr->next;
	}
	return 0;
}

/*Function receives pointer to a FILE and the memory structure and calls the parsing function
 * with a line from the assembly file.  */
int fileReaderAndParser(FILE * pFile, dataStructure *memoryPtr)
{
	char line[MAXLINE+1], *linePtr = NULL;
	int /*length,*/ counter, error = 0, numLine = 0;

	while(fgets(line, MAXLINE+1, pFile) != NULL)
	{

		numLine++;
		linePtr = line;
		counter = 0;
		while (*linePtr!='\r' && *linePtr!='\n' && *linePtr!='\0') /*I had problems with the carriage-return char
																	so I added '\r' to the while statement
																	to mark an end of line*/
		{
			linePtr++;
			counter++;
		}

		line[counter] = '\0';
		error = parser(line, memoryPtr);
		if (error<-99)
		{

			addToErrorlist(numLine, error, memoryPtr);
		}
	}
	return error;
}

/* Function frees all the allocated memory allocated during runtime. */
int cleanUp(dataStructure *memoryPtr)
{
	symbolNode  *tempSymbolPtr=NULL;
	errorListNode* tempErrorPtr = NULL;

	while (memoryPtr->symbolListHead!=NULL)
	{
		tempSymbolPtr = memoryPtr->symbolListHead;
		memoryPtr->symbolListHead = memoryPtr->symbolListHead->next;
		free(tempSymbolPtr);
	}
	while (memoryPtr->errorListHead!=NULL)
	{
		tempErrorPtr = memoryPtr->errorListHead;
		memoryPtr->errorListHead = memoryPtr->errorListHead->next;
		free(tempErrorPtr);
	}
	free(memoryPtr);
	return 0;
}
