#include "validation.h"

/*Validates of the addressing method currently pointed to by str is of direct or index addressing
 * and reports back the addressing information in variable "report"  */
int checkDirectOrIndexAddressing(char *str, char **endPtr, parserReport *report, dataStructure *memoryPtr)
{
	char  *tmpPtr = NULL, **tmpPtrPtr = NULL, temporaryLabel[MAXLABEL];
	int  error = 0,  i, foundNum=0, foundSpace =0;
	int   value, counter=-1;
	char twosCompliment[MAXWORD], *twosComplimentPtr = NULL, **twosComplimentPtrPtr = NULL;

	tmpPtr =str;
	tmpPtrPtr = &str;
	tmpPtrPtr = &tmpPtr;
	skip_spaces(tmpPtr, tmpPtrPtr);
	tmpPtr = str = *tmpPtrPtr;
	if (!(isalpha(*tmpPtr)))
		return illegal_label;
	while (isalpha(*tmpPtr) || isdigit(*tmpPtr))
	{
		tmpPtr++;
		counter++;
	}
	strncpy(temporaryLabel, str, counter+1);
	temporaryLabel[counter+1] = '\0';
	error =  checkLegalLabel(temporaryLabel);
	if (error>=0)
		return 0;
	strcpy(report->values[0], temporaryLabel);
	report->firstValue = 2;
	report->numValues = 1;
	report->addressingMethod =1;
	if (isspace(*tmpPtr))
	{
		foundSpace = 1;
		while (*tmpPtr!='\0' && *tmpPtr!='\n')
		{
			if (!isspace(*tmpPtr))
				return text_after_legal_input;
			tmpPtr++;
		}
	}
	if (*tmpPtr == '[')
	{
		counter = 0;
		str = ++tmpPtr;
		counter = checkLegalDecimal(tmpPtr);
		if (counter>0)
		{
			if (*(tmpPtr+counter) != ']')
				return illegal_label;
			foundNum =1;
			twosComplimentPtr = &twosCompliment[0];
			twosComplimentPtrPtr = &twosComplimentPtr;
			value = atoi(tmpPtr);
			convertDecimalToTwosCompliment(value, twosComplimentPtrPtr);
			for (i = 1; i< MAXWORD -3; i++)
				twosCompliment[i] = twosCompliment[i+2];
			twosCompliment[MAXWORD-2]='0';
			twosCompliment[MAXWORD-3]='0';
			strcpy(report->values[1], twosCompliment);
			report->secondValue = 1;
			report->numValues = 2;
			report->addressingMethod = 2;
		}
		if (foundNum == 0 && foundSpace == 0)
		{
			if (!(isalpha(*tmpPtr))) return illegal_label;
			counter = 0;
			while (isalpha(*tmpPtr) || isdigit(*tmpPtr))
			{
				tmpPtr++;
				counter++;
			}
			if (*tmpPtr != ']')
				return illegal_label;
			strncpy(temporaryLabel, str, counter);
			temporaryLabel[counter] = '\0';
			strcpy(report->values[1], temporaryLabel);
			report->secondValue = 2;
			report->numValues = 2;
			report->addressingMethod = 2;
		}
	}
	if (report->addressingMethod==1 || report->addressingMethod==2) return report->addressingMethod;
	return 0;
}

/*Checks if the assembler file "fileName.as" exists in the current directory*/
FILE * checkIfCodeFileExists(char *fileName)
{
	char filePath[MAXPATH];
	int length;
	FILE * pFile = NULL;

	length = strlen(fileName);
	strcpy(filePath, fileName);
	filePath[length]='.';
	filePath[length+1]='a';
	filePath[length+2]='s';
	filePath[length+3]='\0';
	pFile = fopen(filePath,"r");
	if (pFile==NULL)
		return NULL;
	else
		return (pFile);
	return 0;
}

/*Validates of the addressing method currently pointed to by str is of immediate addressing method
 * and reports back the addressing information in variable "report"  */
int checkImmediateAddressing(char *str, char **endPtr, parserReport *report, dataStructure *memoryPtr)
{
	char  *tmpPtr = NULL, **tmpPtrPtr = NULL;
	int  error = 0, numChars = 0, i;
	int labelType, labelAddress, value;
	char twosCompliment[MAXWORD], *twosComplimentPtr = NULL, **twosComplimentPtrPtr = NULL;
	char label[MAXLABEL], *labelPtr = NULL, **labelPtrPtr = NULL;

	tmpPtr =str;
	tmpPtrPtr = &str;
	tmpPtrPtr = &tmpPtr;
	skip_spaces(tmpPtr, tmpPtrPtr);
	tmpPtr = *tmpPtrPtr;
	if (*tmpPtr != '#')
		return -1;
	if (*tmpPtr == '#')
	{
		tmpPtr++;
		numChars = checkLegalDecimal(tmpPtr);
		if (numChars==illegal_integer_sytnax)
			return illegal_integer_sytnax;
		if (numChars>0)
		{
			str = tmpPtr;
			tmpPtr = tmpPtr + numChars;
			while (*tmpPtr != '\0' && *tmpPtr != '\n')
			{
					if (!isspace(*tmpPtr) ) return bad_addressing_method;
					tmpPtr++;
			}
			value = atoi(str);
			if (value > MAX12BITDECIMALBOUNDS || value < MIN12BITDECIMALBOUNDS)
								return number_out_of_bounds;
			twosComplimentPtr = &twosCompliment[0];
			twosComplimentPtrPtr = &twosComplimentPtr;
			convertDecimalToTwosCompliment(value, twosComplimentPtrPtr);
			for (i = 1; i< MAXWORD -3; i++)
				twosCompliment[i] = twosCompliment[i+2];
			twosCompliment[MAXWORD-2]='0';
			twosCompliment[MAXWORD-3]='0';
			strcpy(report->values[0], twosCompliment);
			report->numValues = 1;
		}
		if (numChars==0)
		{
			labelPtr = &label[0];
			labelPtrPtr = &labelPtr;
			error = extractSymbol(tmpPtr, endPtr, labelPtrPtr);
			if (error==-1)
				return illegal_label;
			error = checkLegalLabel(label);
			if (error>=0)
				return illegal_label;
			error = getLabelData(label, &labelType, &labelAddress, memoryPtr);
			if (error==0) /*label was found*/
			{
				if (labelType!=macro) return immediate_addressing_does_not_use_number_or_macro;
				if (labelAddress > MAX12BITDECIMALBOUNDS || labelAddress < MIN12BITDECIMALBOUNDS)
									return number_out_of_bounds;
				twosComplimentPtr = &twosCompliment[0];
				twosComplimentPtrPtr = &twosComplimentPtr;
				convertDecimalToTwosCompliment(labelAddress, twosComplimentPtrPtr);
				for (i = 1; i< MAXWORD -3; i++)
					twosCompliment[i] = twosCompliment[i+2];
				twosCompliment[MAXWORD-2]='0';
				twosCompliment[MAXWORD-3]='0';
				strcpy(report->values[0], twosCompliment);
				report->numValues = 1;
			}
		}
	}
	if (error==-1)/* label was not found*/
			return immediate_addressing_does_not_use_number_or_macro;
	return 0;
}

/* Receives pointer to string and checks that number is legal (i.e. has a sign with following digits or just digits)
 * returns the number of characters of the number or zero.
 */
int checkLegalDecimal(char *str)
{
	char  *tmpPtr = NULL, *tmpPtr2 = NULL;
	int counter =0 ;

	tmpPtr = str;
	if (*tmpPtr == '+' || *tmpPtr =='-')
	{
		tmpPtr2 = tmpPtr;
		tmpPtr2++;
		if (!isdigit(*tmpPtr2)) return illegal_integer_sytnax;
		tmpPtr++;
		counter++;

	}
	while (isdigit(*tmpPtr))
	{
		tmpPtr++;
		counter++;
	}
	return counter;
}

int checkLegalLabel(char *str)
{
	int i, k = -1 ;

	for (i=0; i < NUMILLEGALNAMES ; i++)
	{
		if ( (strcmp(str, illegalNames[i])) == 0 )
			k = i;
	}

	return k;
}


/*Validates the addressing method currently pointed to by str is of register addressing method
 * and reports back the addressing information in variable "report"  */
int checkRegisterAddressing(char *str, char **endPtr, parserReport *report, dataStructure *memoryPtr)
{
	char  *tmpPtr = NULL, **tmpPtrPtr = NULL;

	tmpPtr =str;
	tmpPtrPtr = &str;
	tmpPtrPtr = &tmpPtr;
	skip_spaces(tmpPtr, tmpPtrPtr);
	str = tmpPtr = *tmpPtrPtr;
	if (*tmpPtr=='r' && (*(tmpPtr+1)>='0' && *(tmpPtr+1)<='7'))
	{
		tmpPtr++;
	}
	else
	{
		return -1;
	}
	report->registerNum = *(str+1) - '0';
	tmpPtr++;
	while (*tmpPtr != '\0' && *tmpPtr != '\n')
	{

		if (!isspace(*tmpPtr)) return illegal_text_after_register;
		tmpPtr++;

	}
		report->addressingMethod = 3;
		strncpy(report->values[0], str, 2);
		report->values[0][2]='\0';
		report->numValues =1;
	return 0;
}

/*checks if "label" exists in the symbol/label list and reports back label data to the caller*/
int checkIfLabelExistsInDataTable(char *label, int *symbolType, int *symbolValue, dataStructure *memoryPtr)
{
	symbolNode *temp = NULL;

	temp = memoryPtr->symbolListHead;
	while (temp!=NULL)
	{
		if (strcmp(label, temp->symbolName)==0)
		{
			*symbolType = temp->symbolType;
			*symbolValue = temp->symbolValue;
			return 1;
		}
		temp=temp->next;
	}
	return 0;
}

/*Checks if a word in the memory table belongs to the type of ".data" or ".string" */
int checkIfLineIsDataOrString(int lineNum, dataStructure *memoryPtr)
{
	symbolNode  *tempSymbolPtr=NULL;
	int foundSymbol = 0;

	tempSymbolPtr = memoryPtr->symbolListHead;
	while (tempSymbolPtr!=NULL && foundSymbol == 0)
	{
		if (tempSymbolPtr->symbolValue == lineNum && tempSymbolPtr->symbolType == data)
		{
			return tempSymbolPtr->numOfLines;
		}
		tempSymbolPtr = tempSymbolPtr->next;
	}
	return 0;
}
