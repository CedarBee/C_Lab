#include "parser.h"

/* Parses the C-string str according to the rules of a ".data" line.
 * Expects str to point to the character right after the word ".data"
 */
int dataParser(char *str, char **endPtr, char **labelPtrPtr, dataStructure *memoryPtr)
{
	int numChars = 0, counter = 0,  endOfLine = 0, foundSign = 0, firstLoop = 0, foundDigit = 0;
	int foundAlpha = 0, spaceAfterData = 0, tmpIC = 0, labelType = 0, labelValue = 0, error =0;
	char tmp2[MAXLINE], tmpString[MAXLINE]="", *tmpPtr = NULL, **tmpPtrPtr = NULL;
	char *tmp=NULL, *firstComma = NULL, *secondComma = NULL;
	long int newNumber = 0;
	char evalString[MAXLINE],  *evalStringPtr = NULL, **evalStringPtrPtr = NULL;
	char emptyString[2] = "", *emptyStringPtr = NULL;
	symbolNode *newSymbolHead = NULL, *tempSymbolPtr=NULL;

	tmpPtr = &tmpString[0];
	tmpPtrPtr = &tmpPtr;
	endPtr = &str;
	skip_spaces(str, endPtr);
	firstComma = str = *endPtr;
	tmpIC = memoryPtr->IC;
	if (*str == ',')
		return comma_before_first_data_input;
	while ((*str != '\0' && *str != '\n') && endOfLine == 0)
	{
		secondComma = strstr(firstComma, ",");
		if (secondComma == NULL)
			{
				while (*str != '\n' && *str != '\0')
					str++;
				secondComma = str;
				endOfLine =1;
			}
		numChars = secondComma - firstComma;
		strncpy(evalString, firstComma, numChars);
		evalString[numChars] = '\0';
		evalStringPtr = evalString;
		evalStringPtrPtr = &evalStringPtr;
		skip_spaces(evalStringPtr, evalStringPtrPtr);
		evalStringPtr = *evalStringPtrPtr;
		tmpPtr = tmp = evalString;
		if ( *evalStringPtr == '\0' || *evalStringPtr == '\n')
			return double_comma;
		while (*evalStringPtr!='\0' && *evalStringPtr != '\n')
			{
				if ( !isalnum(*evalStringPtr) && !isspace(*evalStringPtr) &&
						*evalStringPtr != '\0' && *evalStringPtr != '\n' &&
						*evalStringPtr != '-'  && *evalStringPtr != '+')
					return illegal_integer_sytnax;
			if (firstLoop == 0)
				{
					if ( *evalStringPtr == '\0')
						return double_comma;
					if ( *evalStringPtr == ',')
						return double_comma;
					if ( *evalStringPtr == '-'  || *evalStringPtr == '+' )
					{
						foundSign = 1;
						if (!isdigit(*(evalStringPtr+1))) return illegal_integer_sytnax;
					}
					if ( isdigit(*evalStringPtr) )
						foundDigit = 1;
					if ( isalpha(*evalStringPtr) )
						foundAlpha = 1;
					if (foundSign == 0 && foundDigit == 0 &&  foundAlpha == 0)
						return illegal_integer_sytnax;
				}
				else
				{
					if (*evalStringPtr != '\0' && *evalStringPtr != '\n')
					{
						if ( (*evalStringPtr == '-'  || *evalStringPtr == '+'))
							return illegal_integer_sytnax;
						if ( isalpha(*evalStringPtr) && foundDigit == 1)
							return illegal_integer_sytnax;
						if ( isalpha(*evalStringPtr) && foundSign == 1)
							return illegal_integer_sytnax;
						if (foundAlpha == 1 && !(isalpha(*evalStringPtr) || isdigit(*evalStringPtr)) &&  !isspace(*evalStringPtr))
							return illegal_label;
						if ( spaceAfterData == 1 && !isspace(*evalStringPtr))
							return text_after_legal_input;
					}
					if (isspace(*evalStringPtr)) spaceAfterData = 1;
				}
				if (!isspace(*evalStringPtr))
					counter++;
				evalStringPtr++;
				firstLoop = 1;

			}
			/* add data to memory */
			if (foundSign == 1 || foundDigit ==1)
			{
				newNumber = strtol(tmpPtr, tmpPtrPtr, 10);
				if (newNumber > MAXDECIMALBOUNDS || newNumber < MINDECIMALBOUNDS)
					return number_out_of_bounds;
				convertDecimalToTwosCompliment(newNumber, tmpPtrPtr);
				if (tmpIC >= MAXMEM)
					return can_not_add_to_memory_will_overflow;
				strcpy(memoryPtr->memoryArr[tmpIC++],tmpPtr);
			}
			if (foundAlpha==1)
			{
				skip_spaces(tmpPtr, tmpPtrPtr);
				tmpPtr = *tmpPtrPtr;
				strncpy(tmp2, tmpPtr, counter);
				tmp2[counter]='\0';
				error = getLabelData(tmp2, &labelType, &labelValue, memoryPtr);
				if (error == 0) /*Label was found*/
					{
						if (tmpIC >= MAXMEM)
							return can_not_add_to_memory_will_overflow;
						if (labelType!=macro)
							return label_does_not_refer_to_macro;
						convertDecimalToTwosCompliment(labelValue, tmpPtrPtr);
						strcpy(memoryPtr->memoryArr[tmpIC++], tmpPtr);
					}
				else /*label was not found*/
				{
					return label_has_not_been_defined_yet;
				}

			}
			firstLoop = foundSign = foundDigit = foundAlpha = spaceAfterData = counter = 0;
			if (endOfLine == 0)
				str = firstComma = secondComma+1;
			memoryPtr->DC++;
	}
	if (labelPtrPtr == NULL) /*meaning no label was defined so need to set labelPtrPtr to point to empty string */
	{
		emptyStringPtr = &emptyString[0];
		labelPtrPtr = &emptyStringPtr;
	}
	if (labelPtrPtr != NULL)
	{
		newSymbolHead = newNode(*labelPtrPtr, data, memoryPtr->IC, tmpIC - memoryPtr->IC);
		if (newSymbolHead==NULL)
			return memory_allocation_error;
		if (memoryPtr->symbolListHead==NULL)
			memoryPtr->symbolListHead = newSymbolHead;
		else
		{
			tempSymbolPtr = memoryPtr->symbolListHead;
			while (tempSymbolPtr->next!=NULL)
				tempSymbolPtr = tempSymbolPtr->next;
			tempSymbolPtr->next = newSymbolHead;
		}
	}
	memoryPtr->IC = tmpIC;
	return 0;
}

/* Parses the C-string str according to the rules of a ".define" line.
 * Expects str to point to the character right after the word ".define"
 */
int defineParser(char *str, char **endPtr, char **symbolName, int* symbolValue, dataStructure *memoryPtr)
{
	int counter = 0, symbolType= 0;
	char tmp[MAXLABEL], c, *tmpPtr = NULL;
	symbolNode *newSymbolHead = NULL, *tempSymbolPtr = NULL;

	skip_spaces(str, endPtr);
	str = *endPtr;
	if (!isalpha(*str)) return illegal_macro_label;
	tmpPtr = str;
	c=*tmpPtr;
	while (isalpha(c) || isdigit(c))
	{
		tmpPtr++;
		c=*tmpPtr;
		counter++;
	}
	if (counter > 31) return illegal_macro_label;
	strncpy(tmp, str, counter);
	*(tmp+counter) = '\0';                /*tmp should hold the name of the macro */
	skip_spaces(tmpPtr, endPtr);
	tmpPtr = *endPtr;
	if (*tmpPtr != '=') return illegal_macro_definition;
	tmpPtr++;
	skip_spaces(tmpPtr, endPtr);
	str = tmpPtr = *endPtr; 				/*start of number if syntax checks out */
	c=*tmpPtr;
	if (c == '+' || c == '-')
		{
			tmpPtr++;
			c=*tmpPtr;
		}
	counter = 0;
	while (isdigit(c))
	{
		tmpPtr++;
		c=*tmpPtr;
		counter++;
	}
	skip_spaces(tmpPtr, endPtr);
	c=**endPtr;
	if (counter==0)
		return illegal_macro_value;
	if (c != '\n' && c != '\0')
		return illegal_macro_value;
	*symbolValue = strtol(str, NULL, 10);
	if (checkIfLabelExistsInDataTable(tmp, &symbolType, symbolValue, memoryPtr) == 1)
		return duplicate_macro_definition;
	newSymbolHead = newNode(tmp, macro, *symbolValue, 0);
	if (newSymbolHead==NULL)
		return memory_allocation_error;
	if (memoryPtr->symbolListHead==NULL)
		memoryPtr->symbolListHead = newSymbolHead;
	else
	{
		tempSymbolPtr = memoryPtr->symbolListHead;
		while (tempSymbolPtr->next!=NULL)
			tempSymbolPtr = tempSymbolPtr->next;
		tempSymbolPtr->next = newSymbolHead;
	}
	return 0;
}

/* Parses the C-string str according to the rules of a ".entry" line.
 * Expects str to point to the character right after the word ".entry"
 */
int entryParser(char *str, char **endPtr, char **labelPtrPtr, dataStructure *memoryPtr)
{
	int counter = 0, symbolType = 0, symbolValue = 0, error = 0;
	char *temp = NULL;
	char label[MAXLABEL];
	symbolNode *newSymbolHead = NULL, *tempSymbolPtr=NULL;

	skip_spaces(str, endPtr);
	temp = str = *endPtr;
	if (!isalpha(*str))
		return entry_refers_to_illegal_label_name;
	while (isalnum(*str))
	{
		str++;
		counter++;
	}
	while (*str!='\0' && *str!='\n')
	{
		if (!isspace(*str))
			return text_after_legal_input;
		str++;
	}
	strncpy(label, temp, counter);
	label[counter] = '\0';
	error= checkIfLabelExistsInDataTable(label, &symbolType, &symbolValue, memoryPtr);
	if (error == 1 && symbolType == entry)
		return duplicate_entry_definition;
	newSymbolHead = newNode(label, entry, 99, 0);
	if (newSymbolHead==NULL)
		return memory_allocation_error;
	if (memoryPtr->symbolListHead==NULL)
		memoryPtr->symbolListHead = newSymbolHead;
	else
	{
		tempSymbolPtr = memoryPtr->symbolListHead;
		while (tempSymbolPtr->next!=NULL)
			tempSymbolPtr = tempSymbolPtr->next;
		tempSymbolPtr->next = newSymbolHead;
	}
	return error;
}

/* Parses the C-string str according to the rules of a ".extern" line.
 * Expects str to point to the character right after the word ".extern"
 */
int externParser(char *str, char **endPtr, char **labelPtrPtr, dataStructure *memoryPtr)
{
	int counter = 0, symbolType = 0, symbolValue = 0, error = 0;
	char *temp = NULL;
	char label[MAXLABEL];
	symbolNode *newSymbolHead = NULL, *tempSymbolPtr=NULL;

	skip_spaces(str, endPtr);
	temp = str = *endPtr;
	if (!isalpha(*str))
		return entry_refers_to_illegal_label_name;
	while (isalnum(*str))
	{
		str++;
		counter++;
	}
	while (*str!='\0' && *str!='\n' && *str!='\r')
	{
		if (!isspace(*str))
			return text_after_legal_input;
		str++;
	}
	strncpy(label, temp, counter);
	label[counter] = '\0';
	error= checkIfLabelExistsInDataTable(label, &symbolType, &symbolValue, memoryPtr);
	if (error == 1 && symbolType == external)
		return duplicate_entry_definition;
	newSymbolHead = newNode(label, external, 0, 0);
	if (newSymbolHead==NULL)
		return memory_allocation_error;
	if (memoryPtr->symbolListHead==NULL)
		memoryPtr->symbolListHead = newSymbolHead;
	else
	{
		tempSymbolPtr = memoryPtr->symbolListHead;
		while (tempSymbolPtr->next!=NULL)
			tempSymbolPtr = tempSymbolPtr->next;
		tempSymbolPtr->next = newSymbolHead;
	}
	return error;
}


/* Parses the C-string str. Checks if the current pointer points to a legal syntax label and stores it in 'label'
 * endPtr shall point to the char after the label.
 */
int extractLabel(char *str, char **endPtr, char **label)
{
	int counter = -1, i;
	char *tmp = NULL;

	tmp = str;
	if (!isalpha(*str)) return illegal_label;
	str++;
	counter = 1;
	while (*str != ':')
	{
		if (!(isalpha(*str)) && !(isdigit(*str)))
			return illegal_label;
		str++;
		counter++;

	}
	if (counter >= MAXLABEL) return label_too_long;
	str++; /*to point to the char after ';' */
	strncpy(*label, tmp, counter);
	(*label)[counter]='\0';
	for (i=0; i < NUMILLEGALNAMES ; i++)
	{
		if ( (strstr(*label, illegalNames[i])) != NULL )
			return illegal_label;
	}
	*endPtr = str;
	return counter;
}

/* Parses the C-string str. Checks if the current pointer points to a legal syntax symbol and stores it in 'label'
 * endPtr shall point to the char after the symbol.
 * Difference between symbol and label is that label is the definition and symbol is usage of a label in a command
 */
int extractSymbol(char *str, char **endPtr, char **label)
{
	int counter = -1, i;
	char *tmp = NULL;

	tmp = str;
	if (!isalpha(*str)) return illegal_label;
	str++;
	counter = 1;
	while ((isalpha(*str)) || (isdigit(*str)))
	{
		if (!(isalpha(*str)) && !(isdigit(*str))) return illegal_label;
		str++;
		counter++;
	}
	if (counter >= MAXLABEL) return label_too_long;
	str++; /*to point to the char after ';' */
	strncpy(*label, tmp, counter);
	(*label)[counter]='\0';
	for (i=0; i < NUMILLEGALNAMES ; i++)
	{
		if ( (strstr(*label, illegalNames[i])) != NULL )
			return illegal_label;
	}
	*endPtr = str;
	return counter;
}

/* Parses the C-string str with the commands "rts" or "stop" according to the syntax rules for the commands..
 * Expects str to point to the character right after the command in the parsed line
 * commandCode is the command code number (as in assembler.h)
 * label holds the label name in the begining of the line if exists
 * labelStatus indicates if label exists.
 */
int rtsStopParser(char *str, char **endPtr, dataStructure *memoryPtr, int commandCode,
		char *label, int labelStatus)
{
	int symbolType = -1, symbolValue= -1;
	char *temp;
	symbolNode *newSymbolHead = NULL, *tempSymbolPtr=NULL;

	temp = str;
	while (*temp!='\0' && *temp!='\n')
	{
		if (!isspace(*temp))
			return extraneous_text_after_command;
		temp++;
	}
	if (memoryPtr->IC >= MAXMEM)
		return can_not_add_to_memory_will_overflow;
	if (labelStatus==1)
	{
		if (checkIfLabelExistsInDataTable(label, &symbolType, &symbolValue, memoryPtr)==1)
			if (symbolType == code )
				return double_label_definition;
		newSymbolHead = newNode(label, code, memoryPtr->IC, 1);
		if (newSymbolHead==NULL)
			return memory_allocation_error;
		if (memoryPtr->symbolListHead==NULL)
			memoryPtr->symbolListHead = newSymbolHead;
		else
		{
			tempSymbolPtr = memoryPtr->symbolListHead;
			while (tempSymbolPtr->next!=NULL)
				tempSymbolPtr = tempSymbolPtr->next;
			tempSymbolPtr->next = newSymbolHead;
		}
	}
	if (commandCode == 14)
	{
		strcpy(memoryPtr->memoryArr[memoryPtr->IC++], "00001110000000\0");
	}
	if (commandCode == 15)
	{
		strcpy(memoryPtr->memoryArr[memoryPtr->IC++], "00001111000000\0");
	}
	return 0;
}

/* Parses the C-string str with the commands "prn", "not", "clr", "inc", "dec", "red"
 * according to the syntax rules for the commands.
 * Expects str to point to the character right after the command in the parsed line
 * commandCode is the command code number (as in assembler.h)
 * label holds the label name in the beginning of the line if exists
 * labelStatus indicates if label exists.
 * reportFirstOperand sends back to the calling function the structure of the operand and the
 * "words" found based on the addressing method.
 */
int prnNotClrIncDecRedParser(char *str, char **endPtr, dataStructure *memoryPtr, int commandCode,
		char *label, int labelStatus, parserReport *reportFirstOperand)
{
	int i, error = 0, tempIC=-1, numWords = 0;
	char firstWord[MAXWORD], secondWord[MAXWORD], thirdWord[MAXWORD];
	int firstWordStatus = -1, secondWordStatus = -1, thirdWordStatus = -1;
	int symbolType, symbolValue, labelType, labelAddress, firstSymbol = 0;
	symbolNode *newSymbolHead = NULL, *tempSymbolPtr=NULL;
	char twosCompliment[MAXWORD], *twosComplimentPtr = NULL, **twosComplimentPtrPtr = NULL;

	strcpy(firstWord,"00000000000000\0");
	strcpy(secondWord,"00000000000000\0");
	strcpy(thirdWord,"00000000000000\0");
	error = operandParser(str, endPtr, reportFirstOperand, memoryPtr);
	if (error <-100) return error;
	tempIC = memoryPtr->IC;
	if (reportFirstOperand->addressingMethod == 0)
	{
		if (commandCode!=12)
			return wrong_addressing_method_for_command;
		strcpy(firstWord,"00001100000000\0");
		strcpy(secondWord,reportFirstOperand->values[0]);
		firstWordStatus = 1;
		secondWordStatus = 1;
	}
	if (reportFirstOperand->addressingMethod == 1 || reportFirstOperand->addressingMethod == 2)
	{
		if (reportFirstOperand->addressingMethod == 2 && (commandCode == 9 || commandCode == 10 ||
				commandCode == 13)) return wrong_addressing_method_for_command;
		firstSymbol = checkIfLabelExistsInDataTable(
				reportFirstOperand->values[0], &symbolType, &symbolValue, memoryPtr);
		if (firstSymbol==1)
		{
			if (symbolType==macro)
				return can_not_use_macro_in_direct_addressing;
			if (symbolType!=macro)
			{
				if (symbolValue > MAX12BITDECIMALBOUNDS || symbolValue < MIN12BITDECIMALBOUNDS)
					return number_out_of_bounds;
				twosComplimentPtr = &twosCompliment[0];
				twosComplimentPtrPtr = &twosComplimentPtr;
				convertDecimalToTwosCompliment(symbolValue, twosComplimentPtrPtr);
				for (i = 1; i< MAXWORD -3; i++)
					twosCompliment[i] = twosCompliment[i+2];
				if (symbolType == external)
				{
					twosCompliment[MAXWORD-2]='1';
					twosCompliment[MAXWORD-3]='0';
				}
				else
				{
					twosCompliment[MAXWORD-2]='0';
					twosCompliment[MAXWORD-3]='1';
				}
				strcpy(firstWord,"00001101000100\0");
				strcpy(secondWord,twosCompliment);
				firstWordStatus = 1;
				secondWordStatus = 1;
			}
		}
		else /*add to second pass list */
		{
			strcpy(firstWord,"00001101000100\0");
			firstWordStatus = 1;
			strcpy(secondWord,reportFirstOperand->values[0]);
			secondWordStatus = 2;
		}
	}
	if (reportFirstOperand->addressingMethod == 2)
	{
		if (reportFirstOperand->secondValue==1)
		{
			strcpy(thirdWord,reportFirstOperand->values[1]);
			thirdWordStatus = 1;
		}
		if (reportFirstOperand->secondValue==2)
		{
			error =  getLabelData(reportFirstOperand->values[1], &labelType, &labelAddress, memoryPtr);
			if (labelType!=macro)
				return immediate_addressing_does_not_use_number_or_macro;
			if (labelAddress > MAX12BITDECIMALBOUNDS || labelAddress< MIN12BITDECIMALBOUNDS)
				return number_out_of_bounds;
			twosComplimentPtr = &twosCompliment[0];
			twosComplimentPtrPtr = &twosComplimentPtr;
			convertDecimalToTwosCompliment(labelAddress, twosComplimentPtrPtr);
			for (i = 1; i< MAXWORD -3; i++)
				twosCompliment[i] = twosCompliment[i+2];
			twosCompliment[MAXWORD-2]='0';
			twosCompliment[MAXWORD-3]='0';
			strcpy(thirdWord,twosCompliment);
			thirdWordStatus = 1;
			strcpy(firstWord,"00001101001000\0");
		}
	}
	if (reportFirstOperand->addressingMethod == 3)
	{
		strcpy(firstWord,"00000000001100\0");
		firstWordStatus = 1;
		UpdateRegisterAddress(secondWord, reportFirstOperand->registerNum, 1);
		secondWordStatus = convertedToBinary;
		secondWordStatus = 1;
	}
	if (!(firstWordStatus!= -1)) numWords++;
	if (!(secondWordStatus!= -1)) numWords++;
	if (!(thirdWordStatus!= -1)) numWords++;
	if (!(((memoryPtr->IC) + numWords)<MAXMEM)) return can_not_add_to_memory_will_overflow;
	if (commandCode == 4)
	{
		firstWord[4] = '0';
		firstWord[5] = '1';
		firstWord[6] = '0';
		firstWord[7] = '0';
	}
	if (commandCode == 5)
	{
		firstWord[4] = '0';
		firstWord[5] = '1';
		firstWord[6] = '0';
		firstWord[7] = '1';
	}
	if (commandCode == 7)
	{
		firstWord[4] = '0';
		firstWord[5] = '1';
		firstWord[6] = '1';
		firstWord[7] = '1';
	}
	if (commandCode == 8)
	{
		firstWord[4] = '1';
		firstWord[5] = '0';
		firstWord[6] = '0';
		firstWord[7] = '0';
	}
	if (commandCode == 9)
	{
		firstWord[4] = '1';
		firstWord[5] = '0';
		firstWord[6] = '0';
		firstWord[7] = '1';
	}
	if (commandCode == 10)
	{
		firstWord[4] = '1';
		firstWord[5] = '0';
		firstWord[6] = '1';
		firstWord[7] = '0';
	}
	if (commandCode == 11)
	{
		firstWord[4] = '1';
		firstWord[5] = '0';
		firstWord[6] = '1';
		firstWord[7] = '1';
	}
	if (commandCode == 13)
	{
		firstWord[4] = '1';
		firstWord[5] = '1';
		firstWord[6] = '0';
		firstWord[7] = '1';
	}
	strcpy(memoryPtr->memoryArr[memoryPtr->IC++],firstWord);
	if (reportFirstOperand->addressingMethod == 1 || reportFirstOperand->addressingMethod == 2)
	{
		newSymbolHead = newNode(reportFirstOperand->values[0], symbolInLine, memoryPtr->IC, 0);
		if (newSymbolHead==NULL) return memory_allocation_error;
		if (memoryPtr->symbolListHead==NULL)
			memoryPtr->symbolListHead = newSymbolHead;
		else
		{
			tempSymbolPtr = memoryPtr->symbolListHead;
			while (tempSymbolPtr->next!=NULL)
				tempSymbolPtr = tempSymbolPtr->next;
			tempSymbolPtr->next = newSymbolHead;
		}
	}
	if (secondWordStatus==2)
	{
		memoryPtr->IC++;
	}
	else
	{
		strcpy(memoryPtr->memoryArr[memoryPtr->IC++],secondWord);
	}
	if (thirdWordStatus!=-1)
		strcpy(memoryPtr->memoryArr[memoryPtr->IC++],thirdWord);

	if (labelStatus==1)
	{
		newSymbolHead = newNode(label, code, tempIC, 0);
		if (newSymbolHead==NULL) return memory_allocation_error;
		if (memoryPtr->symbolListHead==NULL)
			memoryPtr->symbolListHead = newSymbolHead;
		else
		{
			tempSymbolPtr = memoryPtr->symbolListHead;
			while (tempSymbolPtr->next!=NULL)
				tempSymbolPtr = tempSymbolPtr->next;
			tempSymbolPtr->next = newSymbolHead;
		}
	}
	return 0;
}

/* Parses the C-string str according to the rules of a ".string" line.
 * Expects str to point to the character right after the word ".string"
 */
int stringParser(char *str, char **endPtr, char **labelPtrPtr, dataStructure *memoryPtr)
{
	char *tmpPtr = NULL, tmpBinary[MAXWORD], *tmpBinaryPtr = NULL,
			**tmpBinaryPtrPtr =NULL;
	int i, counter = 0;
	symbolNode *newSymbolHead = NULL, *tempSymbolPtr=NULL;
	char emptyString[2] = "", *emptyStringPtr = NULL;

	tmpBinaryPtr = &tmpBinary[0];
	tmpBinaryPtrPtr = &tmpBinaryPtr;
	skip_spaces(str, endPtr);
	str = *endPtr;
	if (*str!='"')
		return illegal_string_definition;
	tmpPtr = ++str; /*should point to the beginning of the string*/
	while (*str!='"')
	{
		if (!isprint(*str))
			return illegal_string_definition;
		str++;
		counter++;
	}
	str++;
	while (*str != '\n' && *str != '\0') /*checks if there is garbage after the string*/
	{
		if (!isspace(*str))
			return illegal_string_definition;
		str++;
	}
	if (((memoryPtr->IC)+counter+1) >= MAXMEM)
		return can_not_add_to_memory_will_overflow;
	memoryPtr->DC = (memoryPtr->DC)+counter+1;
	if (labelPtrPtr == NULL) /*meaning no label was defined so need to set labelPtrPtr to point to empty string */
	{
		emptyStringPtr = &emptyString[0];
		labelPtrPtr = &emptyStringPtr;
	}
	if (labelPtrPtr != NULL)
	{
		newSymbolHead = newNode(*labelPtrPtr, data, memoryPtr->IC, counter+1);
		if (newSymbolHead==NULL)
			return memory_allocation_error;
		if (memoryPtr->symbolListHead==NULL)
			memoryPtr->symbolListHead = newSymbolHead;
		else
		{
			tempSymbolPtr = memoryPtr->symbolListHead;
			while (tempSymbolPtr->next!=NULL)
				tempSymbolPtr = tempSymbolPtr->next;
			tempSymbolPtr->next = newSymbolHead;
		}
	}
	for (i = 0; i < counter; i++)
	{
		convertAsciiToBinary(*tmpPtr, tmpBinaryPtrPtr);
		strcpy(memoryPtr->memoryArr[memoryPtr->IC], tmpBinary);
		tmpPtr++;
		(memoryPtr->IC)++;
	}
	convertAsciiToBinary('\0', tmpBinaryPtrPtr);
	strcpy(memoryPtr->memoryArr[memoryPtr->IC], tmpBinary);
	(memoryPtr->IC)++;

	return 0;

}

/*expects to receive str after the command text. can have spaces before and after. ends parsing at
 * null termination or new line termination
 * reports back to calling function with *report
 * or returns an error . -1 if didn't find operand
 */
int operandParser(char *str, char **endPtr, parserReport *report, dataStructure *memoryPtr)
{

	int  error = 0;
	int  foundAddressingMethod = -1;

	error = checkImmediateAddressing(str, endPtr, report, memoryPtr);
	if (error == 0)
		report->addressingMethod = foundAddressingMethod = 0 ;
	if (error<-100)
		return error;
	if (foundAddressingMethod == -1)
	{
		error = checkDirectOrIndexAddressing(str, endPtr, report, memoryPtr);
		if (error <-100 )
			return error;
		if (error !=0)
			foundAddressingMethod = report->addressingMethod = report->numValues;
	}
	if (foundAddressingMethod == -1)
	{
		error = checkRegisterAddressing(str, endPtr, report, memoryPtr);
		foundAddressingMethod = report->addressingMethod;
	}
	return foundAddressingMethod;
}


/* Parses the C-string str with the commands "mov", "cmp", "add", "sub", "lea"
 * according to the syntax rules for the commands.
 * Expects str to point to the character right after the command in the parsed line
 * commandCode is the command code number (as in assembler.h)
 * label holds the label name in the beginning of the line if exists
 * labelStatus indicates if label exists.
 * reportFirstOperand sends back to the calling function the structure of the first operand and the
 * "words" found based on the addressing method.
 * reportSecondOperand sends back to the calling function the structure of the second operand and the
 * "words" found based on the addressing method.
 */
int movCmpAddSubLeaParser(char *str, char **endPtr, dataStructure *memoryPtr, int commandCode,
		char *label, int labelStatus,  parserReport *reportFirstOperand,  parserReport *reportSecondOperand)
{

	int i, error = 0, tempIC=-1, numWords = 0;
	char firstWord[MAXWORD], secondWord[MAXWORD], thirdWord[MAXWORD], fourthWord[MAXWORD] , fifthWord[MAXWORD];
	int firstWordStatus = -1, secondWordStatus = -1, thirdWordStatus = -1, fourthWordStatus = -1, fifthWordStatus = -1;
	int symbolType, symbolValue, labelType, labelAddress, firstSymbol = 0, secondSymbol = 0;
	char *tmpPtr;
	char tempLine[MAXLINE];
	symbolNode *newSymbolHead = NULL, *tempSymbolPtr=NULL;
	char twosCompliment[MAXWORD], *twosComplimentPtr = NULL, **twosComplimentPtrPtr = NULL;

	strcpy(firstWord,"00000000000000\0");
	strcpy(secondWord,"00000000000000\0");
	strcpy(thirdWord,"00000000000000\0");
	strcpy(fourthWord,"00000000000000\0");
	strcpy(fifthWord,"00000000000000\0");
	tmpPtr = str;
	while (*tmpPtr!=',' && *tmpPtr!='\0' && *tmpPtr!='\n')
	{
		if (*tmpPtr=='\0' || *tmpPtr=='\n')
			return missing_comma;
		tmpPtr++;
	}
	strncpy(tempLine, str, tmpPtr-str);
	tempLine[tmpPtr-str] = '\0';
	error = operandParser(tempLine, endPtr, reportFirstOperand, memoryPtr);
	if (error <-100)
		return error;
	tmpPtr++;
	str = tmpPtr;
	while (*tmpPtr!='\0' && *tmpPtr!='\n')
		tmpPtr++;
	strncpy(tempLine, str, tmpPtr-str);
	tempLine[tmpPtr-str] = '\0';
	error = operandParser(tempLine, endPtr, reportSecondOperand, memoryPtr);
	if (error <-100)
		return error;
	tempIC = memoryPtr->IC;
	if (reportFirstOperand->addressingMethod == 0)
	{
		if (commandCode!=1)
			return wrong_addressing_method_for_command;
		strcpy(firstWord,"00000000000000\0");
		strcpy(secondWord,reportFirstOperand->values[0]);
		firstWordStatus = convertedToBinary;
		secondWordStatus = convertedToBinary;
	}
	if (reportFirstOperand->addressingMethod == 1 || reportFirstOperand->addressingMethod == 2)
	{
		firstSymbol = checkIfLabelExistsInDataTable(
				reportFirstOperand->values[0], &symbolType, &symbolValue, memoryPtr);
		if (firstSymbol==1)
		{
			if (symbolType==macro)
				return can_not_use_macro_in_direct_addressing;
			if (symbolType!=macro)
			{
				if (symbolValue > MAX12BITDECIMALBOUNDS || symbolValue < MIN12BITDECIMALBOUNDS)
					return number_out_of_bounds;
				twosComplimentPtr = &twosCompliment[0];
				twosComplimentPtrPtr = &twosComplimentPtr;
				convertDecimalToTwosCompliment(symbolValue, twosComplimentPtrPtr);
				for (i = 1; i< MAXWORD -3; i++)
					twosCompliment[i] = twosCompliment[i+2];
				if (symbolType == external)
				{
					twosCompliment[MAXWORD-2]='1';
					twosCompliment[MAXWORD-3]='0';
				}
				else
				{
					twosCompliment[MAXWORD-2]='0';
					twosCompliment[MAXWORD-3]='1';
				}
				strcpy(firstWord,"00000000010000\0");
				strcpy(secondWord,twosCompliment);
				firstWordStatus = convertedToBinary;
				secondWordStatus = convertedToBinary;
			}
		}
		else /*add to second pass list */
		{
			strcpy(firstWord,"00000000010000\0");
			firstWordStatus = convertedToBinary;
			strcpy(secondWord,reportFirstOperand->values[0]);
			secondWordStatus = labelNeedsToBeChecked;
		}
	}
	if (reportFirstOperand->addressingMethod == 2)
	{
		strcpy(firstWord,"00000000100000\0");
		if (reportFirstOperand->secondValue==1)
		{
			strcpy(thirdWord,reportFirstOperand->values[1]);
			thirdWordStatus = convertedToBinary;
		}
		if (reportFirstOperand->secondValue==2)
		{
			error =  getLabelData(reportFirstOperand->values[1], &labelType, &labelAddress, memoryPtr);
			if (labelType!=macro)
				return immediate_addressing_does_not_use_number_or_macro;
			if (labelAddress > MAX12BITDECIMALBOUNDS || labelAddress< MIN12BITDECIMALBOUNDS)
				return number_out_of_bounds;
			twosComplimentPtr = &twosCompliment[0];
			twosComplimentPtrPtr = &twosComplimentPtr;
			convertDecimalToTwosCompliment(labelAddress, twosComplimentPtrPtr);
			for (i = 1; i< MAXWORD -3; i++)
				twosCompliment[i] = twosCompliment[i+2];
			twosCompliment[MAXWORD-2]='0';
			twosCompliment[MAXWORD-3]='0';
			strcpy(thirdWord,twosCompliment);
			thirdWordStatus = convertedToBinary;
		}
	}
	if (reportFirstOperand->addressingMethod == 3)
	{
		strcpy(firstWord,"00000000110000\0");
		firstWordStatus = convertedToBinary;
		twosComplimentPtr = &twosCompliment[0];
		twosComplimentPtrPtr = &twosComplimentPtr;
		UpdateRegisterAddress(secondWord, reportFirstOperand->registerNum, 0);
		secondWordStatus = convertedToBinary;
	}
/*now starting to figure out second operand */
	if (reportSecondOperand->addressingMethod == 0)
	{
		if (commandCode==6)
			return wrong_addressing_method_for_command;
		strcpy(fourthWord,reportSecondOperand->values[0]);
		fourthWordStatus = convertedToBinary;
	}
	if (reportSecondOperand->addressingMethod == 1 || reportSecondOperand->addressingMethod == 2)
	{
		secondSymbol = checkIfLabelExistsInDataTable(
				reportSecondOperand->values[0], &symbolType, &symbolValue, memoryPtr);
		if (secondSymbol ==1)
		{
			if (symbolType==macro)
				return can_not_use_macro_in_direct_addressing;
			if (symbolType!=macro)
			{
				if (symbolValue > MAX12BITDECIMALBOUNDS || symbolValue < MIN12BITDECIMALBOUNDS)
					return number_out_of_bounds;
				twosComplimentPtr = &twosCompliment[0];
				twosComplimentPtrPtr = &twosComplimentPtr;
				convertDecimalToTwosCompliment(symbolValue, twosComplimentPtrPtr);
				for (i = 1; i< MAXWORD -3; i++)
					twosCompliment[i] = twosCompliment[i+2];
				if (symbolType == external)
				{
					twosCompliment[MAXWORD-2]='1';
					twosCompliment[MAXWORD-3]='0';
				}
				else
				{
					twosCompliment[MAXWORD-2]='0';
					twosCompliment[MAXWORD-3]='1';
				}
				firstWord[11] = '1';
				strcpy(fourthWord,twosCompliment);
				fourthWordStatus = convertedToBinary;
			}
		}
		else /*add to second pass list */
		{
			firstWord[11] = '1';
			strcpy(fourthWord,reportSecondOperand->values[0]);
			fourthWordStatus = labelNeedsToBeChecked;
		}
	}
	if (reportSecondOperand->addressingMethod == 2)
	{
		firstWord[10] = '1';
		firstWord[11] = '0';
		if (reportSecondOperand->secondValue==1)
		{
			strcpy(fifthWord,reportSecondOperand->values[1]);
			fifthWordStatus = convertedToBinary;
		}
		if (reportSecondOperand->secondValue==permanentIndex)
		{
			error =  getLabelData(reportSecondOperand->values[1], &labelType, &labelAddress, memoryPtr);
			if (labelType!=macro)
				return immediate_addressing_does_not_use_number_or_macro;
			if (labelAddress > MAX12BITDECIMALBOUNDS || labelAddress< MIN12BITDECIMALBOUNDS)
				return number_out_of_bounds;
			twosComplimentPtr = &twosCompliment[0];
			twosComplimentPtrPtr = &twosComplimentPtr;
			convertDecimalToTwosCompliment(labelAddress, twosComplimentPtrPtr);
			for (i = 1; i< MAXWORD -3; i++)
				twosCompliment[i] = twosCompliment[i+2];
			twosCompliment[MAXWORD-2]='0';
			twosCompliment[MAXWORD-3]='0';
			strcpy(fifthWord,twosCompliment);
			fifthWordStatus = convertedToBinary;
		}
	}
	if (reportSecondOperand->addressingMethod == directRegister)
	{
		firstWord[11]='1';
		firstWord[10]='1';
		twosComplimentPtr = &twosCompliment[0];
		twosComplimentPtrPtr = &twosComplimentPtr;
		UpdateRegisterAddress(secondWord, reportSecondOperand->registerNum, 1);
		secondWordStatus = convertedToBinary;
	}
	if (!(firstWordStatus!= -1)) numWords++;
	if (!(secondWordStatus!= -1)) numWords++;
	if (!(thirdWordStatus!= -1)) numWords++;
	if (!(fourthWordStatus!= -1)) numWords++;
	if (!(fifthWordStatus!= -1)) numWords++;
	if (!(((memoryPtr->IC) + numWords)<MAXMEM)) return can_not_add_to_memory_will_overflow;
	if (commandCode == 0)
	{
		firstWord[4] = '0';
		firstWord[5] = '0';
		firstWord[6] = '0';
		firstWord[7] = '0';
	}
	if (commandCode == 1)
	{
		firstWord[4] = '0';
		firstWord[5] = '0';
		firstWord[6] = '0';
		firstWord[7] = '1';
	}
	if (commandCode == 2)
	{
		firstWord[4] = '0';
		firstWord[5] = '0';
		firstWord[6] = '1';
		firstWord[7] = '0';
	}
	if (commandCode == 3)
	{
		firstWord[4] = '0';
		firstWord[5] = '0';
		firstWord[6] = '1';
		firstWord[7] = '1';
	}
	if (commandCode == 6)
	{
		firstWord[4] = '1';
		firstWord[5] = '0';
		firstWord[6] = '0';
		firstWord[7] = '1';
	}
	if (commandCode == 10)
	{
		firstWord[4] = '0';
		firstWord[5] = '1';
		firstWord[6] = '0';
		firstWord[7] = '0';
	}
	strcpy(memoryPtr->memoryArr[memoryPtr->IC++],firstWord);
	if (reportFirstOperand->addressingMethod == 1 || reportFirstOperand->addressingMethod == 2)
	{
		newSymbolHead = newNode(reportFirstOperand->values[0], symbolInLine, memoryPtr->IC, 0);
		if (newSymbolHead==NULL)
			return memory_allocation_error;
		if (memoryPtr->symbolListHead==NULL)
			memoryPtr->symbolListHead = newSymbolHead;
		else
		{
			tempSymbolPtr = memoryPtr->symbolListHead;
			while (tempSymbolPtr->next!=NULL)
				tempSymbolPtr = tempSymbolPtr->next;
			tempSymbolPtr->next = newSymbolHead;
		}
	}
	if (secondWordStatus==labelNeedsToBeChecked)
		memoryPtr->IC++;
	if (secondWordStatus==convertedToBinary)
		strcpy(memoryPtr->memoryArr[memoryPtr->IC++],secondWord);
	if (thirdWordStatus!=-1)
		strcpy(memoryPtr->memoryArr[memoryPtr->IC++],thirdWord);
	if (labelStatus==1)
	{
		newSymbolHead = newNode(label, code, tempIC, 0);
		if (newSymbolHead==NULL)
			return memory_allocation_error;
		if (memoryPtr->symbolListHead==NULL)
			memoryPtr->symbolListHead = newSymbolHead;
		else
		{
			tempSymbolPtr = memoryPtr->symbolListHead;
			while (tempSymbolPtr->next!=NULL)
				tempSymbolPtr = tempSymbolPtr->next;
			tempSymbolPtr->next = newSymbolHead;
		}
	}
	if (reportSecondOperand->addressingMethod == 1 || reportSecondOperand->addressingMethod == 2)
	{
		newSymbolHead = newNode(reportSecondOperand->values[0], symbolInLine, memoryPtr->IC, 0);
		if (newSymbolHead==NULL)
			return memory_allocation_error;
		if (memoryPtr->symbolListHead==NULL)
			memoryPtr->symbolListHead = newSymbolHead;
		else
		{
			tempSymbolPtr = memoryPtr->symbolListHead;
			while (tempSymbolPtr->next!=NULL)
				tempSymbolPtr = tempSymbolPtr->next;
			tempSymbolPtr->next = newSymbolHead;
		}
	}
	if (fourthWordStatus==labelNeedsToBeChecked)
		memoryPtr->IC++;
	if (fourthWordStatus==convertedToBinary)
		strcpy(memoryPtr->memoryArr[memoryPtr->IC++],fourthWord);
	if (fifthWordStatus!=-1)
		strcpy(memoryPtr->memoryArr[memoryPtr->IC++],fifthWord);

	return 0;
}

/*Parses an assembler line "str" according to the syntax rules*/
int parser(char *str, dataStructure *memoryPtr)
{

	int i, k = -1, error = 0,  commandCode =-1, labelStatus = 0, value, symbolType=-1, symbolValue=-1;
	char  *tmpPtr = NULL, **endPtr = NULL;
	char label[MAXLABEL], *labelPtr = NULL, **labelPtrPtr = NULL, emptyLabel[2] ="", *emptyLabelPtr=NULL;
	parserReport *reportFirstOperand = NULL;
	parserReport *reportSecondOperand = NULL;

	reportFirstOperand = (parserReport *)malloc(sizeof(parserReport));
	if (reportFirstOperand==NULL)
		return memory_allocation_error;
	reportFirstOperand->firstValue = 0;
	reportFirstOperand->numValues = 0;
	reportFirstOperand->secondValue=0;
	reportFirstOperand->addressingMethod = -1;
	reportSecondOperand = (parserReport *)malloc(sizeof(parserReport));
	if (reportSecondOperand==NULL)
		return memory_allocation_error;
	reportSecondOperand->firstValue = 0;
	reportSecondOperand->numValues = 0;
	reportSecondOperand->secondValue=0;
	reportSecondOperand->addressingMethod = -1;
	emptyLabelPtr = &emptyLabel[0];
	labelPtrPtr = &emptyLabelPtr;
	endPtr = &str;
	skip_spaces(str, endPtr); /* Skips the first whitespaces*/
	str = *endPtr;
	if (*str == ';' || *str == '\0' || *str == '\n')
		return empty_line;
	for (i=0; i < NUMILLEGALNAMES ; i++)
	{
		if ( (tmpPtr = strstr(str, illegalNames[i])) == str )
			k = i;
	}
	if (k == -1) /*There is probably a Label in the beginning of the line*/
	{
		labelPtr = &label[0];
		labelPtrPtr = &labelPtr;
		error = extractLabel(str, endPtr, labelPtrPtr);
		if (error < 0 )
			return illegal_label;
		labelStatus = 1; /*label exists and need to add to table if no error */
	}
	str = *endPtr; /*str should point to the character after a label or to the same place if there was no label */
	skip_spaces(str, endPtr);
	str = *endPtr;
	if (labelStatus == 1)
	{
		if (checkIfLabelExistsInDataTable(label, &symbolType, &symbolValue, memoryPtr)==1)
			if (symbolType == code )
				return double_label_definition;
	}
	error = 0;
	if ( strstr(str, "rts") == str )
	{
		commandCode = 14;
		str = str+3;
		error = rtsStopParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if ( strstr(str, "stop") == str )
	{
		commandCode = 15;
		str = str+4;
		error = rtsStopParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if ( strstr(str, ".entry") == str )
	{
		str = str+6;
		error = entryParser(str, endPtr, labelPtrPtr, memoryPtr);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if ( strstr(str, ".extern") == str )
	{
		str = str+7;
		error = externParser(str, endPtr, labelPtrPtr, memoryPtr);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "mov")==str)
	{
		str = str+3;
		commandCode = 0;
		error = movCmpAddSubLeaParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand, reportSecondOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;

	}
	if (strstr(str, "cmp")==str)
	{
		str = str+3;
		commandCode = 1;
		error = movCmpAddSubLeaParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand, reportSecondOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "add")==str)
	{
		str = str+3;
		commandCode = 2;
		error = movCmpAddSubLeaParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand, reportSecondOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "sub")==str)
	{
		str = str+3;
		commandCode = 3;
		error = movCmpAddSubLeaParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand, reportSecondOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "lea")==str)
	{
		str = str+3;
		commandCode = 6;
		error = movCmpAddSubLeaParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand, reportSecondOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "prn")==str)
	{
		str = str+3;
		commandCode = 12;
		error = prnNotClrIncDecRedParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "not")==str)
	{
		str = str+3;
		commandCode = 4;
		error = prnNotClrIncDecRedParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "clr")==str)
	{
		str = str+3;
		commandCode = 5;
		error = prnNotClrIncDecRedParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "inc")==str)
	{
		str = str+3;
		commandCode = 7;
		error = prnNotClrIncDecRedParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "dec")==str)
	{
		str = str+3;
		commandCode = 8;
		error = prnNotClrIncDecRedParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "jmp")==str)
	{
		str = str+3;
		commandCode = 9;
		error = prnNotClrIncDecRedParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "bne")==str)
	{
		str = str+3;
		commandCode = 10;
		error = prnNotClrIncDecRedParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "red")==str)
	{
		str = str+3;
		commandCode = 11;
		error = prnNotClrIncDecRedParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if (strstr(str, "jsr")==str)
	{
		str = str+3;
		commandCode = 13;
		error = prnNotClrIncDecRedParser(str, endPtr, memoryPtr, commandCode,
				label, labelStatus, reportFirstOperand);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if ( strstr(str, ".define") == str )
	{
		if (k == -1)
			{
			return illegal_label_before_macro;
			}
		str = str+7;
		error = defineParser(str, endPtr, labelPtrPtr, &value, memoryPtr);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if ( strstr(str, ".string") == str )
	{
		str = str+7;
		error = stringParser(str, endPtr, labelPtrPtr, memoryPtr);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	if ( strstr(str, ".data") == str )
	{
		str = str+5;
		error = dataParser(str, endPtr, labelPtrPtr, memoryPtr);
		free(reportFirstOperand);
		free(reportSecondOperand);
		return error;
	}
	free(reportFirstOperand);
	free(reportSecondOperand);

	return unknown_command;
}


/* Parses the C-string str. Skips white spaces until encountering a non white space character.
 * endptr will point to the first non-whitespace character, or null if reached end of line
 * Function returns number of whitespace characters skipped.
 */
int skip_spaces(char *str, char **endptr)
{
	char c;
	int counter = 0;

	c = *str;
	*(endptr) = (str);

	while (isspace(c) && c != '\n' && c != '\0')
	{
		str++;
		c = *str;
		*(endptr) = (str);
		counter++;
	}
	return counter;
}

/*Updates the secondWord with the relevant binary word based on the register number
 * firstOrSecondRegister shall be 0 or 1, for first and second appropriately */
int UpdateRegisterAddress(char *secondWord, int registerNum, int firstOrSecondRegister)
{
		switch(registerNum)
		{
		case 0:
			secondWord[8+firstOrSecondRegister*3]='0';
			secondWord[7+firstOrSecondRegister*3]='0';
			secondWord[6+firstOrSecondRegister*3]='0';
		break;
		case 1:
			secondWord[8+(firstOrSecondRegister*3)]='1';
			secondWord[7+(firstOrSecondRegister*3)]='0';
			secondWord[6+(firstOrSecondRegister*3)]='0';
		break;
		case 2:
			secondWord[8+firstOrSecondRegister*3]='0';
			secondWord[7+firstOrSecondRegister*3]='1';
			secondWord[6+firstOrSecondRegister*3]='0';
		break;
		case 3:
			secondWord[8+firstOrSecondRegister*3]='1';
			secondWord[7+firstOrSecondRegister*3]='1';
			secondWord[6+firstOrSecondRegister*3]='0';
		break;
		case 4:
			secondWord[8+firstOrSecondRegister*3]='0';
			secondWord[7+firstOrSecondRegister*3]='0';
			secondWord[6+firstOrSecondRegister*3]='1';
		break;
		case 5:
			secondWord[8+firstOrSecondRegister*3]='1';
			secondWord[7+firstOrSecondRegister*3]='0';
			secondWord[6+firstOrSecondRegister*3]='1';
		break;
		case 6:
			secondWord[8+firstOrSecondRegister*3]='0';
			secondWord[7+firstOrSecondRegister*3]='1';
			secondWord[6+firstOrSecondRegister*3]='1';
		break;
		case 7:
			secondWord[8+firstOrSecondRegister*3]='1';
			secondWord[7+firstOrSecondRegister*3]='1';
			secondWord[6+firstOrSecondRegister*3]='1';
		break;
		}

	return 0;

}
