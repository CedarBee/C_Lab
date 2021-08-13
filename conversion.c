#include "conversion.h"

/*Converts decimal ascii code to binary */
int convertAsciiToBinary(int asciiCode, char **convertedBinary)
{
	char binNum[MAXWORD], tmp[MAXWORD];
	int i;

	for (i = 0; i < MAXWORD-1 ; i++)
		binNum[i] = '0';
	binNum[MAXWORD-1] = '\0';
	for (i = 0; i < MAXWORD-1 ; i++)
		tmp[i] = '0';
	tmp[MAXWORD-1] = '\0';
	for (i = 0; i < (MAXWORD-1) && asciiCode!=0; i++)
	{
		if ((asciiCode % 2) == 1)
			binNum[i]='1';
		else
			binNum[i]='0';

		asciiCode=asciiCode / 2;
	}
	for (i = 0; i < (MAXWORD-1) ; i++) /*reverses the binary word and stores it in convertedBinary */
		tmp[i] = (binNum[MAXWORD-2-i]);
	strcpy(*convertedBinary, tmp);

	return 0;
}

int convertDecimalToTwosCompliment(long int decimal, char **twosCompliment)
{
	char binNum[MAXWORD], tmp[MAXWORD];
	int i, carry = 0, negative = 0 ;

	if (decimal < 0 )
	{
		decimal = decimal * (-1);
		negative = 1;

	}
	for (i = 0; i < MAXWORD-1 ; i++)
		binNum[i] = '0';
	binNum[MAXWORD-1] = '\0';
	for (i = 0; i < MAXWORD-1 ; i++)
		tmp[i] = '0';
	tmp[MAXWORD-1] = '\0';
	for (i = 0; i < (MAXWORD-1) && decimal!=0; i++)
	{
		if ((decimal % 2) == 1)
			binNum[i]='1';
		else
			binNum[i]='0';

		decimal=decimal / 2;
	}
	for (i = 0; i < (MAXWORD-1) ; i++) /*reverses the binary word and stores it in convertedBinary */
		tmp[i] = (binNum[MAXWORD-2-i]);
	/* Converted to binary. Now converts to two's compliment */
	if (negative == 1)
	{
		for (i = 0; i < (MAXWORD-1) ; i++)
		{
			if (tmp[i] == '0')
			{
				tmp[i] = '1';
			}
			else
			{
				tmp[i] = '0';
			}
		}

		if (tmp[MAXWORD-2] == '0')
		{
			tmp[MAXWORD-2] = '1';
		}
		else
		{
			tmp[MAXWORD-2] = '0';
			carry = 1;
			for (i = MAXWORD-3; i >= 0 ; i--)
			{
					if (carry == 0)
					{
							if (tmp[i] == '0')
							{
								tmp [i] = '0';
								carry = 0;
							}
							else
							{
								tmp [i] = '1';
								carry = 0;
							}
					}
				else
					{
							if (tmp[i] == '0')
							{
								tmp[i] = '1';
								carry = 0;
							}
							else
							{
								tmp[i] = '0';
								carry = 1;
							}
					}
			}
		}

	}
	strcpy(*twosCompliment, tmp);
	return 0;
}

/*Converts the 14 letter memory word to the special base 4 characters and terminates with null character */
void convertLineTo4Bit(char **fourBitLinePtrPtr, char *lineToConvert)
{
	int i, j=0;

	for (i=1 ; i<=13 ; i+=2)
	{
		if (lineToConvert[i-1] == '0' && lineToConvert[i] == '0')
			(*fourBitLinePtrPtr)[j] = '*';
		if (lineToConvert[i-1] == '0' && lineToConvert[i] == '1')
			(*fourBitLinePtrPtr)[j] = '#';
		if (lineToConvert[i-1] == '1' && lineToConvert[i] == '0')
			(*fourBitLinePtrPtr)[j] = '%';
		if (lineToConvert[i-1] == '1' && lineToConvert[i] == '1')
			(*fourBitLinePtrPtr)[j] = '!';
		j++;
	}
	(*fourBitLinePtrPtr)[7]='\0';
}
