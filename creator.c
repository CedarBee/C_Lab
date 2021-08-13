#include "creator.h"

/*Function initializes a dataStructure type and returns pointer to the caller */
dataStructure *initDataStructure(void)
{
	dataStructure *tmp = NULL;

	tmp = (dataStructure *)malloc(sizeof(dataStructure));
	if (tmp == NULL)
		return NULL;
	tmp->DC = 0;
	tmp->IC = 100;
	tmp->symbolListHead = NULL;
	tmp->errorListHead = NULL;

	return tmp;
}

/*Creates new node of symbol list and populates with input
 * Returns pointer to new node */
symbolNode* newNode(char *symbolName, int symbolType, int symbolValue, int numOfLines)
{
	symbolNode *tmp=NULL;

	tmp = (symbolNode *)malloc(sizeof(symbolNode));

	if(tmp!= NULL)
	{
			if (symbolName==NULL)
				strcpy(tmp->symbolName, "");
			else
				strcpy(tmp->symbolName, symbolName);
			tmp->symbolType = symbolType;
			tmp->symbolValue = symbolValue;
			tmp->numOfLines = numOfLines;
			tmp->next = NULL;
	}
	return tmp;
}

/*Creates the "object" file and populates with the code*/
int createObjectFile(char *fileName, dataStructure *memoryPtr)
{
	int i, jumpLines=0, lineCounter = 100;
	char fullFileName[MAXPATH];
	char fourBitLine[8], *fourBitLinePtr = NULL, **fourBitLinePtrPtr = NULL;
	symbolNode  *tempSymbolPtr=NULL;
	FILE *fp=NULL;

	fourBitLinePtr = &fourBitLine[0];
	fourBitLinePtrPtr = &fourBitLinePtr;

	strcpy(fullFileName, fileName);
	strcat(fullFileName,".ob");

	if (memoryPtr->IC>100)
	{
		fp = fopen(fullFileName, "w");
		fprintf(fp, "\t%d %d\n", (memoryPtr->IC)-(memoryPtr->DC)-100, memoryPtr->DC);

		for (i=100; i<memoryPtr->IC; i++) /*create code translation */
		{
			jumpLines = 0;
			jumpLines = checkIfLineIsDataOrString(i, memoryPtr);
			i=i+jumpLines;
			if (i<memoryPtr->IC && jumpLines == 0)
			{
				convertLineTo4Bit(fourBitLinePtrPtr, memoryPtr->memoryArr[i]);
				fprintf(fp,"%04d\t%s\n",lineCounter, fourBitLine);
				lineCounter++;
			}
			if (jumpLines>0)
				i--;
		}
		tempSymbolPtr = memoryPtr->symbolListHead;
		while (tempSymbolPtr!=NULL)
		{
			if (tempSymbolPtr->symbolType == data)
			{
				for (i=tempSymbolPtr->symbolValue; (i<(tempSymbolPtr->numOfLines)+(tempSymbolPtr->symbolValue)); i++)
				{
					convertLineTo4Bit(fourBitLinePtrPtr, memoryPtr->memoryArr[i]);
					fprintf(fp,"%04d\t%s\n",lineCounter, fourBitLine);
					lineCounter++;
				}
			}
			tempSymbolPtr = tempSymbolPtr->next;
		}
		fclose(fp);

		return 0;
	}
	return no_object_file_created;
}

/*Creates the "entry" file and populates with the code*/
int createEntryFile(char *fileName, dataStructure *memoryPtr)
{
	int fileStatus = 0;
	char fullFileName[MAXPATH];
	symbolNode  *tempSymbolPtr=NULL, *tempSymbolSecondLoopPtr=NULL;
	FILE *fp=NULL;

	tempSymbolPtr = memoryPtr->symbolListHead;
	while (tempSymbolPtr!=NULL)
	{
		if (tempSymbolPtr->symbolType == entry)
		{
			tempSymbolSecondLoopPtr = memoryPtr->symbolListHead;
			while (tempSymbolSecondLoopPtr!=NULL)
			{
				if (strcmp(tempSymbolPtr->symbolName, tempSymbolSecondLoopPtr->symbolName)==0
						&& (tempSymbolSecondLoopPtr->symbolType==code ||
								tempSymbolSecondLoopPtr->symbolType==data))
				{
					if (fileStatus == 0)
					{
						strcpy(fullFileName, fileName);
						strcat(fullFileName,".ent");
						fp = fopen(fullFileName, "w");
						fileStatus = 1;
					}
					fprintf(fp,"%s  %04d\n", tempSymbolSecondLoopPtr->symbolName, tempSymbolSecondLoopPtr->symbolValue);
				}
				tempSymbolSecondLoopPtr = tempSymbolSecondLoopPtr->next;
			 }
		 }
		 tempSymbolPtr = tempSymbolPtr->next;
	}
	if (fileStatus == 1)
	{
		fclose(fp);
		return 0;
	}
	return no_entry_file_created;
}

/*Creates the "external" file and populates with the code*/
int createExternalFile(char *fileName, dataStructure *memoryPtr)
{
	int fileStatus = 0;
	char fullFileName[MAXPATH];
	symbolNode  *tempSymbolPtr=NULL, *tempSymbolSecondLoopPtr=NULL;
	FILE *fp=NULL;

	tempSymbolPtr = memoryPtr->symbolListHead;
	while (tempSymbolPtr!=NULL)
	{
		if (tempSymbolPtr->symbolType == external)
		{
			tempSymbolSecondLoopPtr = memoryPtr->symbolListHead;
			while (tempSymbolSecondLoopPtr!=NULL)
			{
				if (strcmp(tempSymbolPtr->symbolName, tempSymbolSecondLoopPtr->symbolName)==0
						&& (tempSymbolSecondLoopPtr->symbolType==code ||
								tempSymbolSecondLoopPtr->symbolType==data ||
								tempSymbolSecondLoopPtr->symbolType==symbolInLine))
				{
					if (fileStatus == 0)
					{
						strcpy(fullFileName, fileName);
						strcat(fullFileName,".ext");
						fp = fopen(fullFileName, "w");
						fileStatus = 1;
					}
					fprintf(fp,"%s  %04d\n", tempSymbolSecondLoopPtr->symbolName, tempSymbolSecondLoopPtr->symbolValue);
				}
				tempSymbolSecondLoopPtr = tempSymbolSecondLoopPtr->next;
			 }
		 }
		 tempSymbolPtr = tempSymbolPtr->next;
	}
	if (fileStatus == 1)
	{
		fclose(fp);
		return 0;
	}
	return no_external_file_created;
}
