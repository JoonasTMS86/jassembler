#define errUNKNOWNINSTRUCTION 1
#define errVALUEOUTOFRANGE 2
#define errBRANCHOUTOFRANGE 3
#define errSYNTAXERROR 4
#define errENDOFFILE 5
#define errVALUENOTDEFINED 6
#define errUNKNOWNDIRECTIVE 7
#define errFILEINCLUDEERROR 8
#define errINVALIDFILLNUMBER 9

#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace std;

char * loadedFile = (char*) malloc(1920138);
char * isaFile = (char*) malloc(1920138);
char * savedFile = (char*) malloc(1920138);
unsigned int loadedSize = 0;
unsigned int savedSize;
unsigned int isaSize = 0;
int cError; // Compiler error code
int currentLine; // Current line on the source file
int byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7;
int valueStackPushPointer;
int valueStackPopPointer;
int valueStack[100];
bool checkingSourceInstruction;
bool invalidValueFindAlternativeInstruction;
string allowedBitWidth;
int isaPos;
int isaLine;
int isaMnemonicPos;
int sourcePos;
int sourceLineOffset;
bool instructionFound;
int numericValue;
int CPUAddress; // Current address of CPU. Its value can be changed with the ORG directive.
string lineContent;
int variablesSize = 0;
string variableNames[1000000];
int variableValues[1000000];
string nameFoundAtLine;
bool valueNotDefined;
bool possibleError;
int passes;

const int byteTable[160] = 
{
0x8A,0xC7,0x23,0x04,0x89,0xE8,0x00,0x00,
0x0D,0xE0,0xB6,0xB3,0xA7,0x64,0x00,0x00,
0x01,0x63,0x45,0x78,0x5D,0x8A,0x00,0x00,
0x00,0x23,0x86,0xF2,0x6F,0xC1,0x00,0x00,
0x00,0x03,0x8D,0x7E,0xA4,0xC6,0x80,0x00,
0x00,0x00,0x5A,0xF3,0x10,0x7A,0x40,0x00,
0x00,0x00,0x09,0x18,0x4E,0x72,0xA0,0x00,
0x00,0x00,0x00,0xE8,0xD4,0xA5,0x10,0x00,
0x00,0x00,0x00,0x17,0x48,0x76,0xE8,0x00,
0x00,0x00,0x00,0x02,0x54,0x0B,0xE4,0x00,
0x00,0x00,0x00,0x00,0x3B,0x9A,0xCA,0x00,
0x00,0x00,0x00,0x00,0x05,0xF5,0xE1,0x00,
0x00,0x00,0x00,0x00,0x00,0x98,0x96,0x80,
0x00,0x00,0x00,0x00,0x00,0x0F,0x42,0x40,
0x00,0x00,0x00,0x00,0x00,0x01,0x86,0xA0,
0x00,0x00,0x00,0x00,0x00,0x00,0x27,0x10,
0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xE8,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x64,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
};

void push()
{
	valueStack[valueStackPushPointer] = byte7;
	valueStack[valueStackPushPointer - 1] = byte6;
	valueStack[valueStackPushPointer - 2] = byte5;
	valueStack[valueStackPushPointer - 3] = byte4;
	valueStack[valueStackPushPointer - 4] = byte3;
	valueStack[valueStackPushPointer - 5] = byte2;
	valueStack[valueStackPushPointer - 6] = byte1;
	valueStack[valueStackPushPointer - 7] = byte0;
	valueStackPushPointer -= 8;
}

void pop()
{
	byte7 = valueStack[valueStackPopPointer];
	byte6 = valueStack[valueStackPopPointer - 1];
	byte5 = valueStack[valueStackPopPointer - 2];
	byte4 = valueStack[valueStackPopPointer - 3];
	byte3 = valueStack[valueStackPopPointer - 4];
	byte2 = valueStack[valueStackPopPointer - 5];
	byte1 = valueStack[valueStackPopPointer - 6];
	byte0 = valueStack[valueStackPopPointer - 7];
	valueStackPopPointer -= 8;
}

void writeDigitsAsByte(char digit1, char digit2)
{
	int value = 0;
	// 8-bit value
	if(digit1 == '!')
	{
		pop();
		savedFile[savedSize] = byte0;
		savedSize++;
		CPUAddress++;
	}
	// 16-bit value
	else if(digit1 == '?')
	{
		pop();
		savedFile[savedSize] = byte0;
		savedFile[savedSize + 1] = byte1;
		savedSize += 2;
		CPUAddress += 2;
	}
	// 64-bit value
	else if(digit1 == '"')
	{
		pop();
		savedFile[savedSize] = byte0;
		savedFile[savedSize + 1] = byte1;
		savedFile[savedSize + 2] = byte2;
		savedFile[savedSize + 3] = byte3;
		savedFile[savedSize + 4] = byte4;
		savedFile[savedSize + 5] = byte5;
		savedFile[savedSize + 6] = byte6;
		savedFile[savedSize + 7] = byte7;
		savedSize += 8;
		CPUAddress += 8;
	}
	else
	{
		if(digit1 >= 'A') value = value + ((digit1 - 55) * 16);
		else value = value + ((digit1 - 48) * 16);
		if(digit2 >= 'A') value = value + (digit2 - 55);
		else value = value + (digit2 - 48);
		savedFile[savedSize] = value;
		savedSize++;
		CPUAddress++;
	}
}

void writeByte(int value)
{
	char digit1 = value >> 4;
	char digit2 = value & 15;
	if(digit1 < 10) digit1 += 0x30;
	else digit1 += 0x37;
	if(digit2 < 10) digit2 += 0x30;
	else digit2 += 0x37;
	writeDigitsAsByte(digit1, digit2);
}

void error(string msg)
{
	cout << "Line " << currentLine << ": " << msg << endl;
	free (loadedFile);
	free (savedFile);
	free (isaFile);
}

// Get the hexadecimal value in the source file.
void toHex(int pos)
{
	byte0 = 0;
	int currentDigit = 0;
	bool checking = true;
	while(checking)
	{
		int fourbits;
		int digit1 = -1;
		int digit2 = 0;
		for(int digitsChecked = 0; digitsChecked < 2; digitsChecked++)
		{
			if(loadedFile[pos] >= '0' && loadedFile[pos] <= '9')
			{
				fourbits = loadedFile[pos] - 48;
			}
			else if(loadedFile[pos] >= 'a' && loadedFile[pos] <= 'f')
			{
				fourbits = loadedFile[pos] - 87;
			}
			else if(loadedFile[pos] >= 'A' && loadedFile[pos] <= 'F')
			{
				fourbits = loadedFile[pos] - 55;
			}
			else break;
			if(digitsChecked == 0) digit1 = fourbits;
			else digit2 = fourbits * 16;
			pos--;
		}
		if(digit1 == -1) checking = false;
		else
		{
			switch(currentDigit)
			{
				case 0:
					byte0 = digit1 + digit2;
					break;
				case 1:
					byte1 = digit1 + digit2;
					break;
				case 2:
					byte2 = digit1 + digit2;
					break;
				case 3:
					byte3 = digit1 + digit2;
					break;
				case 4:
					byte4 = digit1 + digit2;
					break;
				case 5:
					byte5 = digit1 + digit2;
					break;
				case 6:
					byte6 = digit1 + digit2;
					break;
				case 7:
					byte7 = digit1 + digit2;
					break;
			}
		}
		currentDigit++;
	}
}

// Get the decimal value in the source file.
void toDecimal(int pos)
{
	byte0 = 0;
	byte1 = 0;
	byte2 = 0;
	byte3 = 0;
	byte4 = 0;
	byte5 = 0;
	byte6 = 0;
	byte7 = 0;
	int byteTablePos = 152;
	while(loadedFile[pos] >= '0' && loadedFile[pos] <= '9')
	{
		int multiplier = loadedFile[pos] - 48;
		while(multiplier > 0)
		{
			int carry;
			byte0 = byte0 + byteTable[byteTablePos + 7];
			if(byte0 >= 256)
			{
				byte0 -= 256;
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			byte1 = byte1 + byteTable[byteTablePos + 6] + carry;
			if(byte1 >= 256)
			{
				byte1 -= 256;
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			byte2 = byte2 + byteTable[byteTablePos + 5] + carry;
			if(byte2 >= 256)
			{
				byte2 -= 256;
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			byte3 = byte3 + byteTable[byteTablePos + 4] + carry;
			if(byte3 >= 256)
			{
				byte3 -= 256;
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			byte4 = byte4 + byteTable[byteTablePos + 3] + carry;
			if(byte4 >= 256)
			{
				byte4 -= 256;
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			byte5 = byte5 + byteTable[byteTablePos + 2] + carry;
			if(byte5 >= 256)
			{
				byte5 -= 256;
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			byte6 = byte6 + byteTable[byteTablePos + 1] + carry;
			if(byte6 >= 256)
			{
				byte6 -= 256;
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			byte7 = byte7 + byteTable[byteTablePos + 0] + carry;
			if(byte7 >= 256)
			{
				byte7 -= 256;
			}
			multiplier--;
		}
		byteTablePos -= 8;
		pos--;
	}
}

void doNumericValue(bool lobyte, bool hibyte)
{
	if(byte0 == -1 || valueNotDefined)
	{
	}
	else
	{
		if(lobyte)
		{
			byte1 = 0;
			byte2 = 0;
			byte3 = 0;
			byte4 = 0;
			byte5 = 0;
			byte6 = 0;
			byte7 = 0;
		}
		if(hibyte)
		{
			if(byte7 != 0) byte0 = byte7;
			else if(byte6 != 0) byte0 = byte6;
			else if(byte5 != 0) byte0 = byte5;
			else if(byte4 != 0) byte0 = byte4;
			else if(byte3 != 0) byte0 = byte3;
			else if(byte2 != 0) byte0 = byte2;
			else if(byte1 != 0) byte0 = byte1;
			byte1 = 0;
			byte2 = 0;
			byte3 = 0;
			byte4 = 0;
			byte5 = 0;
			byte6 = 0;
			byte7 = 0;
		}
		numericValue = byte0 + (byte1 * 256) + (byte2 * 65536) + (byte3 * 16777216);
	}
}

// Evaluate the expression.
int evaluateExpression(int evaluateExpressionMnemonicPos)
{
	numericValue = -1;
	bool lobyte = false;
	bool hibyte = false;
	valueNotDefined = false;
	// If the number being checked is not a valid decimal or hexadecimal value, then byte0 is -1.
	int pos = evaluateExpressionMnemonicPos;
	int origPos = evaluateExpressionMnemonicPos;
	if(loadedFile[pos] == '<')
	{
		lobyte = true;
		pos++;
		origPos++;
	}
	else if(loadedFile[pos] == '>')
	{
		hibyte = true;
		pos++;
		origPos++;
	}
	byte0 = -1;
	byte1 = 0;
	byte2 = 0;
	byte3 = 0;
	byte4 = 0;
	byte5 = 0;
	byte6 = 0;
	byte7 = 0;
	if(loadedFile[pos] == '$' || loadedFile[pos] == '0' && loadedFile[pos + 1] == 'x')
	{
		// Hexadecimal value found.
		if(loadedFile[pos] == '$') pos++;
		else pos += 2;
		while((loadedFile[pos] >= '0' && loadedFile[pos] <= '9') || (loadedFile[pos] >= 'a' && loadedFile[pos] <= 'f') || 
(loadedFile[pos] >= 'A' && loadedFile[pos] <= 'F'))
		{
			pos++;
		}
		evaluateExpressionMnemonicPos = pos;
		pos--;
		toHex(pos);
	}
	else if(loadedFile[pos] >= '0' && loadedFile[pos] <= '9')
	{
		// Decimal value found.
		while(loadedFile[pos] >= '0' && loadedFile[pos] <= '9') pos++;
		evaluateExpressionMnemonicPos = pos;
		pos--;
		toDecimal(pos);
	}
	else
	{
		byte0 = -1;
	}
	if(byte0 != -1)
	{
		doNumericValue(lobyte, hibyte);
		return evaluateExpressionMnemonicPos;
	}
	else
	{
		valueNotDefined = true;
		nameFoundAtLine = "";
		int pos = origPos;
		while(loadedFile[pos] != 10 && loadedFile[pos] != 13 && loadedFile[pos] != 32 && loadedFile[pos] != ',' && loadedFile[pos] != ';')
		{
			nameFoundAtLine = nameFoundAtLine + loadedFile[pos];
			pos++;
		}
				evaluateExpressionMnemonicPos = pos;
		pos = 0;
		if(nameFoundAtLine == "*")
		{
			byte0 = CPUAddress & 0xFF;
			byte1 = (CPUAddress >> 8) & 0xFF;
			byte2 = (CPUAddress >> 16) & 0xFF;
			byte3 = (CPUAddress >> 24) & 0xFF;
			valueNotDefined = false;
			doNumericValue(lobyte, hibyte);
			return evaluateExpressionMnemonicPos;
		}
		else
		{
			while(pos < variablesSize)
			{
				if(nameFoundAtLine == variableNames[pos])
				{
					byte0 = variableValues[pos] & 0xFF;
					byte1 = (variableValues[pos] >> 8) & 0xFF;
					byte2 = (variableValues[pos] >> 16) & 0xFF;
					byte3 = (variableValues[pos] >> 24) & 0xFF;
					valueNotDefined = false;
					doNumericValue(lobyte, hibyte);
					return evaluateExpressionMnemonicPos;
				}
				pos++;
			}
		}
	}
	return evaluateExpressionMnemonicPos;
}

// If the currently checked line didn't match any existing instruction, then we go to the next line of the ISA file.
void nextLineOfIsaFile()
{
	sourceLineOffset = sourcePos;
	isaPos = isaMnemonicPos;
	while(isaFile[isaPos] != 13 && isaFile[isaPos] != 10)
	{
		isaPos++;
		if(isaPos >= isaSize) break;
	}
	if((isaPos + 1) < isaSize)
	{
		if(isaFile[isaPos] == 13 && isaFile[isaPos + 1] == 10)
		{
			isaPos++;
		}
	}
	isaPos++;
	if(isaPos >= isaSize)
	{
		checkingSourceInstruction = false;
		possibleError = true;
		if(invalidValueFindAlternativeInstruction) cError = errVALUEOUTOFRANGE;
		if(valueNotDefined)
		{
			if(cError != errVALUEOUTOFRANGE) cError = errVALUENOTDEFINED;
		}
	}
	isaMnemonicPos = isaPos + 78;
	isaLine += 2;
	valueStackPushPointer = 99;
	valueStackPopPointer = 99;
}

void processDirective(string directive)
{
	if(directive == "org")
	{
		while(loadedFile[sourceLineOffset] <= 32) sourceLineOffset++;
		sourceLineOffset = evaluateExpression(sourceLineOffset);
		CPUAddress = numericValue;
	}
	else if(directive == "byte")
	{
		bool checking = true;
		while(checking)
		{
			while(loadedFile[sourceLineOffset] == 32) sourceLineOffset++;
			if(loadedFile[sourceLineOffset] == 10 || loadedFile[sourceLineOffset] == 13 || loadedFile[sourceLineOffset] == ';')
			{
				checking = false;
				if(loadedFile[sourceLineOffset] == ';')
				{
					while(loadedFile[sourceLineOffset] != 10 && loadedFile[sourceLineOffset] != 13) sourceLineOffset++;
				}
			}
			else
			{
				while(loadedFile[sourceLineOffset] <= 32) sourceLineOffset++;
				if(loadedFile[sourceLineOffset] == ',')
				{
					sourceLineOffset++;
					while(loadedFile[sourceLineOffset] <= 32) sourceLineOffset++;
				}
				sourceLineOffset = evaluateExpression(sourceLineOffset);
				if(byte0 == -1)
				{
					checkingSourceInstruction = false;
					cError = errSYNTAXERROR;
					checking = false;
				}
				else
				{
					writeByte(byte0);
				}
			}
		}
	}
	else if(directive == "text")
	{
		while(loadedFile[sourceLineOffset] != '"')
		{
			sourceLineOffset++;
			if(sourceLineOffset >= loadedSize)
			{
				checkingSourceInstruction = false;
				cError = errENDOFFILE;
				break;
			}
		}
		sourceLineOffset++;
		while(loadedFile[sourceLineOffset] != '"')
		{
			if(loadedFile[sourceLineOffset] == '\\') sourceLineOffset++;
			writeByte(loadedFile[sourceLineOffset]);
			sourceLineOffset++;
			if(loadedFile[sourceLineOffset] == 13 || loadedFile[sourceLineOffset] == 10 || sourceLineOffset >= loadedSize)
			{
				checkingSourceInstruction = false;
				cError = errENDOFFILE;
				break;
			}
		}
		sourceLineOffset++;
	}
	else if(directive == "bin")
	{
		while(loadedFile[sourceLineOffset] != '"')
		{
			sourceLineOffset++;
			if(sourceLineOffset >= loadedSize)
			{
				checkingSourceInstruction = false;
				cError = errENDOFFILE;
				break;
			}
		}
		sourceLineOffset++;
		char binToLoadName[256];
		int filenamepos = 0;
		while(loadedFile[sourceLineOffset] != '"')
		{
			binToLoadName[filenamepos] = loadedFile[sourceLineOffset];
			sourceLineOffset++;
			filenamepos++;
			if(loadedFile[sourceLineOffset] == 13 || loadedFile[sourceLineOffset] == 10 || sourceLineOffset >= loadedSize)
			{
				checkingSourceInstruction = false;
				cError = errENDOFFILE;
				break;
			}
		}
		binToLoadName[filenamepos] = 0;
		sourceLineOffset++;
		char * binFile = (char*) malloc(1920138);
		int binSize = 0;
		ifstream binToLoad(binToLoadName, ios::in|ios::binary|ios::ate);
		if(binToLoad)
		{
			binSize = binToLoad.tellg();
			binToLoad.seekg (0, ios::beg);
			binToLoad.read (binFile, binSize);
			binToLoad.close();
		}
		else
		{
			cError = errFILEINCLUDEERROR;
			checkingSourceInstruction = false;
		}
		for(int pos = 0; pos < binSize; pos++)
		{
			writeByte(binFile[pos]);
		}
		free (binFile);
	}
	else if(directive == "fill")
	{
		while(loadedFile[sourceLineOffset] <= 32) sourceLineOffset++;
		sourceLineOffset = evaluateExpression(sourceLineOffset); // How many
		int howmany = numericValue;
		while(loadedFile[sourceLineOffset] != ',') sourceLineOffset++;
		sourceLineOffset++;
		while(loadedFile[sourceLineOffset] <= 32) sourceLineOffset++;
		sourceLineOffset = evaluateExpression(sourceLineOffset); // Byte with which to fill
		int bytevalue = numericValue;
		if(howmany == -1 || bytevalue == -1)
		{
			checkingSourceInstruction = false;
			cError = errVALUENOTDEFINED;
		}
		else
		{
		if(howmany < 0)
		{
			checkingSourceInstruction = false;
			cError = errINVALIDFILLNUMBER;
		}
		else if(bytevalue < 0 || bytevalue > 255)
		{
			allowedBitWidth = "8";
			checkingSourceInstruction = false;
			cError = errVALUEOUTOFRANGE;
		}
		else
		{
			while(howmany > 0)
			{
				writeByte(bytevalue);
				howmany--;
			}
		}
		}
	}
	else
	{
		checkingSourceInstruction = false;
		cError = errUNKNOWNDIRECTIVE;
	}
}

void assemble()
{
	valueNotDefined = false;
	possibleError = false;
	CPUAddress = 0;
	currentLine = 1;
	cError = 0;
	savedSize = 0;
	sourcePos = 0;
	while(sourcePos < loadedSize)
	{
		int backToThisOffset = sourcePos;
		// Read a line from the file. First, ignore all the chars of value 32 or less.
		while(loadedFile[sourcePos] < 33) sourcePos++;
		sourceLineOffset = sourcePos;
		checkingSourceInstruction = true;
		isaPos = 0;
		isaLine = 1;
		isaMnemonicPos = isaPos + 78;
		valueStackPushPointer = 99;
		valueStackPopPointer = 99;
		invalidValueFindAlternativeInstruction = false;
		instructionFound = false;
		int checkOffset = sourceLineOffset;
		bool isExpression = false;
		while(checkOffset < loadedSize && loadedFile[checkOffset] != 10 && loadedFile[checkOffset] != 13)
		{
			if(loadedFile[checkOffset] == '=')
			{
				isExpression = true;
				break;
			}
			checkOffset++;
		}
		// Make sure the '=' really signifies expression by checking whether it is preceded by a comment character.
		if(isExpression)
		{
			while(checkOffset > (backToThisOffset - 1))
			{
				if(loadedFile[checkOffset] == ';')
				{
					isExpression = false;
					break;
				}
				checkOffset--;
			}
		}
		if(loadedFile[backToThisOffset] == 10 || loadedFile[backToThisOffset] == 13)
		{
			isExpression = false;
			checkingSourceInstruction = false;
		}
		if(isExpression)
		{
			if(passes == 0)
			{
				checkOffset = sourceLineOffset;
				string nameToAdd = "";
				while(loadedFile[checkOffset] > 32 && loadedFile[checkOffset] != '=')
				{
					nameToAdd = nameToAdd + loadedFile[checkOffset];
					checkOffset++;
				}
				while(loadedFile[checkOffset] != '=') checkOffset++;
				checkOffset++;
				while(loadedFile[checkOffset] <= 32) checkOffset++;
				checkOffset = evaluateExpression(checkOffset);
				variableNames[variablesSize] = nameToAdd;
				variableValues[variablesSize] = numericValue;
				variablesSize++;
				possibleError = false;
				cError = 0;
			}
		}
		else
		{
			while(checkingSourceInstruction)
			{
				if(isaFile[isaMnemonicPos] == 13 || isaFile[isaMnemonicPos] == 10 || isaMnemonicPos >= isaSize)
				{
					if((isaMnemonicPos + 1) < isaSize)
					{
						if(isaFile[isaMnemonicPos] == 13 && isaFile[isaMnemonicPos + 1] == 10)
						{
							isaMnemonicPos++;
						}
					}
					isaMnemonicPos++;
					// Instruction found?
					int savedsourceLineOffset = sourceLineOffset;
					while(loadedFile[sourceLineOffset] == 0 || loadedFile[sourceLineOffset] == 7 || loadedFile[sourceLineOffset] == 32)
					{
						sourceLineOffset++;
					}
					if(loadedFile[sourceLineOffset] == 13 || loadedFile[sourceLineOffset] == 10 || loadedFile[sourceLineOffset] == ';')
					{
						// Instruction found! Now write the opcode & possible param(s) to the output file.
						checkingSourceInstruction = false;
						sourceLineOffset = savedsourceLineOffset;
						instructionFound = true;
						char digit1 = isaFile[isaPos];
						char digit2 = isaFile[isaPos + 1];
						while(digit1 != 32)
						{
							writeDigitsAsByte(digit1, digit2);
							isaPos += 3;
							digit1 = isaFile[isaPos];
							digit2 = isaFile[isaPos + 1];
						}
					}
					else
					{
						// Nope, must keep looking for a completely matching instruction.
						sourceLineOffset = savedsourceLineOffset;
						nextLineOfIsaFile();
					}
				}
				else
				{
					while(loadedFile[sourceLineOffset] <= 32)
					{
						sourceLineOffset++;
					}
					char comparedChar = loadedFile[sourceLineOffset];
					if(comparedChar >= 'a' && comparedChar <= 'z') comparedChar -= 32;
					if(isaFile[isaMnemonicPos] == comparedChar)
					{
						sourceLineOffset++;
						isaMnemonicPos++;
						while(isaFile[isaMnemonicPos] == 32) isaMnemonicPos++;
					}
					// Assembler directive?
					else if(comparedChar == '.')
					{
						sourceLineOffset++;
						string directive = "";
						while(loadedFile[sourceLineOffset] != 0 &&
loadedFile[sourceLineOffset] != 7 &&
loadedFile[sourceLineOffset] != 10 &&
loadedFile[sourceLineOffset] != 13 &&
loadedFile[sourceLineOffset] != 32)
						{
							directive = directive + loadedFile[sourceLineOffset];
							sourceLineOffset++;
						}
						for(int pos = 0; pos < directive.length(); pos++)
						{
							if(directive[pos] >= 'A' && directive[pos] <= 'Z') directive[pos] = directive[pos] + 32;
						}
						processDirective(directive);
						checkingSourceInstruction = false;
					}
					// Comment? Then we ignore all characters except for line break.
					else if(comparedChar == ';')
					{
						checkingSourceInstruction = false;
					}
					else if(isaFile[isaMnemonicPos] == '!')
					{
						/*
						Expecting 8-bit value in the source file.
						If the value is larger than 8 bits, then we go looking for a version of this
						instruction where the value is allowed.
						If no such instruction is found, then we have a "Value out of range" error.
						*/
						isaMnemonicPos += 2;
						sourceLineOffset = evaluateExpression(sourceLineOffset);
						if(byte0 == -1)
						{
							nextLineOfIsaFile();
						}
						if(byte1 != 0 || byte2 != 0 || byte3 != 0 || byte4 != 0 || byte5 != 0 || byte6 != 0 || byte7 != 0)
						{
							invalidValueFindAlternativeInstruction = true;
							allowedBitWidth = "8";
							sourceLineOffset = sourcePos;
							isaPos = isaMnemonicPos;
							while(isaFile[isaPos] != 13 && isaFile[isaPos] != 10)
							{
								isaPos++;
								if(isaPos >= isaSize) break;
							}
							if((isaPos + 1) < isaSize)
							{
								if(isaFile[isaPos] == 13 && isaFile[isaPos + 1] == 10) isaPos++;
							}
							isaPos++;
							if(isaPos >= isaSize)
							{
								checkingSourceInstruction = false;
								possibleError = true;
								if(invalidValueFindAlternativeInstruction) cError = errVALUEOUTOFRANGE;
							}
							isaMnemonicPos = isaPos + 78;
						}
						else push();
					}
					else if(isaFile[isaMnemonicPos] == '?')
					{
						/*
						Expecting 16-bit value in the source file.
						If the value is larger than 16 bits, then we go looking for a version of this
						instruction where the value is allowed.
						If no such instruction is found, then we have a "Value out of range" error.
						*/
						isaMnemonicPos += 2;
						sourceLineOffset = evaluateExpression(sourceLineOffset);
						if(byte0 == -1)
						{
							nextLineOfIsaFile();
						}
						if(byte2 != 0 || byte3 != 0 || byte4 != 0 || byte5 != 0 || byte6 != 0 || byte7 != 0)
						{
							invalidValueFindAlternativeInstruction = true;
							allowedBitWidth = "16";
							sourceLineOffset = sourcePos;
							isaPos = isaMnemonicPos;
							while(isaFile[isaPos] != 13 && isaFile[isaPos] != 10)
							{
								isaPos++;
								if(isaPos >= isaSize) break;
							}
							if((isaPos + 1) < isaSize)
							{
								if(isaFile[isaPos] == 13 && isaFile[isaPos + 1] == 10) isaPos++;
							}
							isaPos++;
							if(isaPos >= isaSize)
							{
								checkingSourceInstruction = false;
								possibleError = true;
								if(invalidValueFindAlternativeInstruction) cError = errVALUEOUTOFRANGE;
							}
							isaMnemonicPos = isaPos + 78;
						}
						else push();
					}
					else if(isaFile[isaMnemonicPos] == '"')
					{
						// Expecting 64-bit value in the source file.
						isaMnemonicPos += 2;
						sourceLineOffset = evaluateExpression(sourceLineOffset);
						if(byte0 == -1)
						{
							nextLineOfIsaFile();
						}
						else push();
					}
					else if(isaFile[isaMnemonicPos] == '@')
					{
						// Value in source file is an 8-bit relative value.
						isaMnemonicPos += 2;
						sourceLineOffset = evaluateExpression(sourceLineOffset);
						if(byte0 == -1)
						{
							if(cError != errVALUEOUTOFRANGE) cError = errVALUENOTDEFINED;
							push();
						}
						else
						{
							int relativeJumpValue = numericValue - CPUAddress - 2;
							byte0 = (relativeJumpValue & 255);
							push();
							if(relativeJumpValue < -128 || relativeJumpValue > 127)
							{
								checkingSourceInstruction = false;
								cError = errBRANCHOUTOFRANGE;
							}
						}
					}
					else if(isaFile[isaMnemonicPos] == '~')
					{
						// Value in source file is a 64-bit relative value.
						isaMnemonicPos += 2;
						sourceLineOffset = evaluateExpression(sourceLineOffset);
						if(byte0 == -1)
						{
							if(cError != errVALUEOUTOFRANGE) cError = errVALUENOTDEFINED;
							push();
						}
						else
						{
							int relativeJumpValue = numericValue - CPUAddress;
							byte0 = relativeJumpValue & 0xFF;
							byte1 = (relativeJumpValue >> 8) & 0xFF;
							byte2 = (relativeJumpValue >> 16) & 0xFF;
							byte3 = (relativeJumpValue >> 24) & 0xFF;
							if(relativeJumpValue < 0)
							{
								byte4 = 0xFF;
								byte5 = 0xFF;
								byte6 = 0xFF;
								byte7 = 0xFF;
							}
							push();
						}
					}
					else
					{
						nextLineOfIsaFile();
					}
				}
			}
		}
		if(possibleError != 0)
		{
			possibleError = false;
			lineContent = "";
			sourceLineOffset = sourcePos;
			while(loadedFile[sourceLineOffset] != 13 && loadedFile[sourceLineOffset] != 10 && sourceLineOffset < loadedSize)
			{
				lineContent = lineContent + loadedFile[sourceLineOffset];
				sourceLineOffset++;
			}
			bool checking = true;
			bool errorTrue = false;
			int pos = 0;
			while(checking)
			{
				if(lineContent[pos] == ' ')
				{
					checking = false;
					errorTrue = true;
				}
				pos++;
				if(pos >= lineContent.length()) checking = false;
			}
			if(!errorTrue)
			{
				if(passes == 0)
				{
					variableNames[variablesSize] = lineContent;
					variableValues[variablesSize] = CPUAddress;
					variablesSize++;
				}
			}
			else
			{
				if(cError == 0) cError = errUNKNOWNINSTRUCTION;
			}
		}
		// Go to the next line of the source file.
		sourcePos = backToThisOffset;
		while(loadedFile[sourcePos] != 13 && loadedFile[sourcePos] != 10) sourcePos++;
		if((sourcePos + 1) < loadedSize)
		{
			if(loadedFile[sourcePos] == 13 && loadedFile[sourcePos + 1] == 10) sourcePos++;
		}
		sourcePos++;
		if(!possibleError && cError == 0) currentLine++;
	}
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		cout << "JAssembler v0.4" << endl;
		cout << "Assemble your source code into any binary format" << endl;
		cout << "defined in the chosen instruction set." << endl;
		cout << endl;
		cout << "(C) 2021 Joonas Lindberg (The Mad Scientist)" << endl;
		cout << endl;
		cout << "Usage: jassembler instructionsetfile sourcefile targetfile" << endl;
		free (loadedFile);
		free (savedFile);
		free (isaFile);
		return 0;
	}
	if(argc < 4)
	{
		cout << "All parameters must be entered." << endl;
		free (loadedFile);
		free (savedFile);
		free (isaFile);
		return 0;
	}
	char * isaFileName = argv[1];
	char * sourceFileName = argv[2];
	char * targetFileName = argv[3];
	ifstream isafile(isaFileName, ios::in|ios::binary|ios::ate);
	if(isafile)
	{
		isaSize = isafile.tellg();
		isafile.seekg (0, ios::beg);
		isafile.read (isaFile, isaSize);
	}
	else
	{
		cout << "\"" << isaFileName << "\" not found!" << endl;
		free (loadedFile);
		free (savedFile);
		free (isaFile);
		return(1);
	}
	isafile.close();
	ifstream sourcefile(sourceFileName, ios::in|ios::binary|ios::ate);
	if(sourcefile)
	{
		loadedSize = sourcefile.tellg();
		sourcefile.seekg (0, ios::beg);
		sourcefile.read (loadedFile, loadedSize);
	}
	else
	{
		cout << "\"" << sourceFileName << "\" not found!" << endl;
		free (loadedFile);
		free (savedFile);
		free (isaFile);
		return(1);
	}
	sourcefile.close();
	// Here we start assembling the source file into binary.
	passes = 0;
	assemble();
	passes++;
	if(cError != 0)
	{
		assemble(); // Second pass, in case there were errors in the first one (such as variables which were not found).
		passes++;
	}
	switch(cError)
	{
		case 0:
			break;
		case errUNKNOWNINSTRUCTION:
			error("Unknown instruction");
			break;
		case errVALUEOUTOFRANGE:
			error("Value out of range - " + allowedBitWidth + "-bit value expected");
			break;
		case errBRANCHOUTOFRANGE:
			error("Target address out of range");
			break;
		case errSYNTAXERROR:
			error("Syntax error");
			break;
		case errENDOFFILE:
			error("Unexpected end of file");
			break;
		case errVALUENOTDEFINED:
			error("Value not defined");
			break;
		case errUNKNOWNDIRECTIVE:
			error("Unknown Assembler directive");
			break;
		case errFILEINCLUDEERROR:
			error("File not found or other file error");
			break;
		case errINVALIDFILLNUMBER:
			error("Parameter 1 for fill must be 0 or greater");
			break;
	}
	if(cError != 0) return (1);
	ofstream savedfile (targetFileName, ios::out|ios::binary|ios::ate);
	if (savedfile.is_open())
	{
		savedfile.write(savedFile, savedSize);
		savedfile.close();
	}
	else 
	{
		cout << "Error creating file!" << endl;
		free (loadedFile);
		free (savedFile);
		free (isaFile);
		return (1);
	}
	free (loadedFile);
	free (savedFile);
	free (isaFile);
	return 0;
}
