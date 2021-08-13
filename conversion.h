#ifndef __CONVERSION_H
#define __CONVERSION_H

#include "assembler.h"
#include "creator.h"
#include "validation.h"
#include "parser.h"

int convertAsciiToBinary(int asciiCode, char **convertedBinary);
int convertDecimalToTwosCompliment(long int decimal, char **twosCompliment);
void convertLineTo4Bit(char **fourBitLinePtrPtr, char *lineToConvert);

#endif
