/*
Situations I know my assembler can't handle:
	Can't handle strings of data with spaces. I noticed this very soon before the time we had to submit the project. Unfortunately I didn't have time to fix it.


Purpose os the assembler: 
to take assembly files (file names) as input, to parse them, check for erros, and to convert into machine language files with entry and extern helper files if relevant.
Output is in the form of user notifications and the above-mentioned output files.

All of the assembler's c files #include this header file.

Overall Summary of how the assembler works:
In short: it basically follows the 'two pass' method described in the maman 14 instructions.

In depth:
Data structures:
	labelNode: linked list to store labels along with the various parameters related to labels.
	dataNode: linked list to store data along with the various parameters related to data.
	externCallList: linked list to store externs with the various parameters related to externs.
	metadata: a struct containing a numerical value that can be passed from function to function. For example: counters and flags.
	Binary is stored in dynamic array of shorts.

*The assembler accepts arguments via command line and accesses them through argv in the main function*.
The main function includes begins with declarations. 
It then continues to make sure that arguments were passed to it beofre moving on.

It then begins a loop over all arguments. During the loop:
	The user is notified of assmply occuring on the specific argument.
	Memory is allocated for data structures.
	The first pass is carried out.
	The error flag is checked. 'continue' loop if error flag is lit and notifies user that output files won't be created for this input file.
	The second pass is carried out.
	Return value of the second pass is checked. If there were errors, skips to freeing stage and notifies user that output files won't be created for this input file.
	Output files are created.
	Memory is freed.
	User is notified that assembly of this file has finished. 
	End of loop.
User is notified the program is exiting.

*First pass*
Memory is allocated for array of pointers to parameter strings
Input ile is opened for reading.
Loop is initiated (in separate function):
	Line is read.
	Checks if reading returned errors.
	Checks if machine is out of memeory (starts at 0. max defined in MAX_MEM as 1023 as defined in the maman).
	Checks if exceeded max lines (starts at 1. max defined as 100 as defined in the maman).
	Checks if line is blank or comment.
	Else:
		If line has a label adds to label table.
		If line contains extern, adds to label tabel.
		Else if line contains data, adds data to data list.
		Else if line contains entry, skips. to be dealt with in second pass.
		Else if line contains action, action word is added to binary array. 
	Paramters are freed.
	End of loop.
Data table and label table addresses are updated to proper addresses now that the final amount of binary words is known.
File is closed.
Array of pointers to params is freed.

*second pass*
Memory is allocated for array of pointers to parameter strings
Input ile is opened for reading.
Loop is initiated (in separate function):
	Line is read.
	Checks if reading returned errors.
	If entry is detected, label table is updated to reflect this.
	Operand words are entered into binary array.
	Paramters are freed.
	End of loop.
Data is added to the end of the binarary array.

*Output*
ob file is created even if empty. will contain zeros as the instruction and data lengths.
extern and entry are created only if needed.
User is notified for each file if it was created or not.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#define MAX_LABEL_LEN 31
#define MAX_IN 128
#define MAX_PARAM_LEN 80
#define ERROR "\x1B[31mError: \x1B[0m"
#define MAX_ACTION_NAME_SIZE 4
#define MAX_REGISTRY 7
#define MAX_REGISTRY_CHAR '7'
#define MAX_OPCODE 15
#define ACTIONS "mov", "cmp","add","sub","not","clr","lea","inc","dec","jmp","bne","red","prn","jsr","rts","stop"
#define WORD 12
#define BINARY_OFFSET 1,3,7,15,31,63,127,255,511,1023,2047,4059
#define REGISTRY 5
#define ABSOLUTE 1
#define RELOCATABLE 3
#define START_ADDRESS 100
#define MAX_MEM 1023
#define MAX_LINES 100
#define RESERVED "data", "string", "entry", "extern", "@r0",  "@r1", "@r2", "@r3", "@r4", "@r5", "@r6", "@r7"
#define DATA_DELIMS " \t ,"
#define DATA ".data"
#define STRING ".string"
#define ENTRY ".entry"
#define EXTERN ".extern"
#define EMPTY_LIST_ADDRESS -1
#define MAX_SUBSTRINGS 3

/*linked list to store labels along with the various parameters related to labels.*/
typedef struct labelNode{
	char label[MAX_LABEL_LEN]; /*string to store label*/ 
	int address; /*to store address wherelable appears*/
	int isCommand; /*flag to represent if label is a command*/
	int isExtern; /*flag to represent if label is external*/
	int isEntry; /*flag to represent if label is entry*/
	struct labelNode *next; /*pointer to next node*/
} labelNode;

/*linked list to store data along with the various parameters related to data.*/
typedef struct dataNode{
	short value; /*to store data*/ 
	int address; /*to store address where data is referenced*/
	int numOrString; /*1 if num, 2 if string, 0 if neither*/
	struct dataNode *next; /*pointer to next node*/
} dataNode;

/*linked list to store externs with the various parameters related to externs.*/
typedef struct externCallList{
  char label[MAX_LABEL_LEN]; /*string to store label*/ 
  int address; /*to store address where extern is referenced*/
  struct externCallList *next; /*pointer to next node*/
} externCallList;


/*struct containing a numerical value that can be passed from function to function. For example: counters and flags.*/
typedef struct metadata{
	int value; /*numerical value*/
} metadata;


/*main function accepts command line arguments and uses them via argv. Always returns 1.*/

/*firstPass.c*/
/*prepares memory for and triggers loop of reading and processing params. Returns pointer to binary array or null in case of error*/
short *firstPass(char path[], labelNode *labelTableHead, dataNode *dataTableHead, short binary[], metadata *errorFlag);
/*loop of reading and processing of input lines. Returns pointer to binary array or null in case of error*/
short *firstPassLoop(FILE *fp, char **params, short binary[], labelNode *labelTableHead, dataNode *dataTableHead, metadata *errorFlag, char path[]);

/*secondPass.c*/
/*prepares memory for and triggers loop of reading and processing params. Returns 0 in case of success and 1 in case of error*/
int secondPass(char path[], labelNode *labelTableHead, dataNode *dataTableHead, short binary[], metadata *instructionLength, metadata *dataLength, externCallList *externCallListHead);
/*loop of reading and processing of input lines.  Returns 0 in case of success and 1 in case of error*/
int secondPassLoop(char **params, FILE *fp, labelNode *labelTableHead, dataNode *dataTableHead, short binary[], metadata *instructionLength, metadata *dataLength, externCallList *externCallListHead);

/*read.c*/
/*reads line from buffer, clean, split into separate params. returns number of params found on line. negative number represents error*/
int readLine(FILE *fp, char *params[], int lineNumber);
/*separates line into params. returns number of params. returns negative for error*/
int separateParams(char *line, char *params[]);
/*checks for extra commas on given line. returns 1 if extra comma found, 0 if not*/
int extraCommas(char line[]);
/*cleans raw line from whitespaces*/
void cleanLine(char *src, char *dest);
/*checks if operands are legitimate. returns number of words that will be needed, including command word. -1 for error*/
int checkOperands(int opcode, char *params[], int currentParam, int numOfParams, int addressMethods[]);

/*tables.c*/
/*Adds label to label table. returns 0 is ran succeeded, 1 if there was an error*/
int addTolabelTable(char param[], labelNode *labelTableHead, int address, int isCommand, int isExtern);
/*adds data to data table. returns amount of words/elements added*/
int addToDataTable(char data[], int numOrString, int address, dataNode *dataTableHead);
/*update label addresses after they are known. always returns 0.*/
int updateLabelTableAddresses(labelNode *labelTableHead, int IC);
/*update data addresses after they are known. always returns 0.*/
int updateDataTableAddresses(dataNode *dataTableHead, int IC);
/*store external label on external list. always returns 1*/
int storeExt(char label[], int address, externCallList *externCallListHead);
/*checks for duplicate labels. returns 1 if found. 0 if not.*/
int checkForDuplicateLabel(char *params[], labelNode *labelTableHead, int numOfParams);
/*free the linked lists*/
void freeLists(externCallList *externCallListHead, labelNode *labelTableHead, dataNode *dataTableHead);
/*adds externals to label table. returns 0 for errors, 1 for succcess*/
int addExternalToLabelTable(char **params, labelNode *labelTableHead, int lineNumber);

/*booleans.c*/
/*checks if we overflowed the max mem limit based on instruction and data counters. returns 1 if yes, 0 if no*/
int memoryCheck(int IC, int DC);
/*checks if memory allocation returned null pointer. dynamic amount of params. first param is number of params, rest are pointers. returns 1 if yes, 0 if no*/
int allocationCheck(int num,...);
/*checks if param is label. returns 1 if yes, 0 if not*/
int label(char param[]);
/*checks if params contain data or string keyword. data returns 1, string reurns 2 and neither returns 0*/
int isData(char *param[], int numOfParams);
/*checks if params contain entry keyword. 1 if true, 0 if false*/
int isEntry(char *param[], int numOfParams);
/*checks if params contain extern keyword. 1 if true, 0 if false*/
int isExtern(char *param[], int numOfParams);
/*checks if destination method is legal for the given opcode. returns numerical value representing the method, or 0 if failed*/
int legalDestMethod(char param[], int opcode);
/*checks if souce method is legal for the given opcode. returns numerical value representing the method, or 0 if failed*/
int legalSourceMethod(char param[], int opcode);
/*checks if param is a registry. returns 1 if yes, 0 if not*/
int isRegistry(char param[]);
/*checks if param is relocatable data. returns 1 if yes, 0 if not*/
int isRelocatable(char param[]);
/*checks if param is relocatable data. returns 1 if yes, 0 if not*/
int isAbsolute(char param[]);
/*checks if given char is space or tab. returns 1 if yes, 0 if no*/
int isWhiteSpace(char c);
/*checks if given param is numerical. returns 1 if yes, 0 if no*/
int isNumeric(char *c);
/*checks for too many quotes. reurns 1 if error if too many are found, 0 if not*/
int extraQuotes(char *line);
/*counts labels on the given line. skips labels in quotes. returns amount of lables found on line*/
int countLabel(char *line);
/*checks if given param is a reserved name. returns 1 if yes, 0 if no*/
int isReservedName(char *name);
/*checks if there are any entries in the label table. returns 1 if yes, 0 if no*/
int existsEntry(labelNode *labelTableHead);
/*checks if there are any externs in the label table. returns 1 if yes, 0 if no*/
int existsExtern(labelNode *labelTableHead);
/*checks if given label is exernal. returns 1 if yes, 0 if no*/
int isLabelExtern(char label[], labelNode *labelTableHead);

/*gettersAndSetters.c*/
int getActionOpcode(char param[]);
/*returns the base 64 char that represents the given value*/
char getBase64(short bin);
/*gets th address of a label, returns -1 if no match found*/
int getLabelAddress(char label[], labelNode *labelTableHead);
/*returns a string with the name of the action based on the opcode given. null if there was an error*/
char *getActionName(int opCode);
/*sets entry labels with entry flag. retuns 0 for errors and number of labels marked if successful*/
int setEntry(char *params[], int currentParam, labelNode *labelTableHead);
/*returns the number of operands permitted for an action. return -1 if illegal opcode is passed*/
int getPermittedOpCount(int opcode);
/*returns the mask needed for the length given*/
short getMask(int length);
/*gets bits based on shift, length of the mask needed, andthe given word that the bits are taken from*/
short getBits(int place, int length, short word);
/*sets bits based on the shift and value given. retuns word with bits set*/
short setBits(int place, int value);

/*binary.c*/
/*adds command word to binary array. returns pointer to updated binary array or null if there was an error*/
short *addCommandWord(int opcode, int sourceAddressMethod, int destAddressMethod, short binary[], int IC, int length);
/*add operand words to binary array. returns amount of words added*/
int addOperandWord(char param[], int IC, short binary[], labelNode *labelTableHead, externCallList *externCallListHead);
/*adds data table to binary array of shorts. returns number ofdata elements added*/
int addDataToBinary(dataNode *dataTableHead, short binary[]);
/*unsigns a number from a character array. If positive, drops the plus sign. If negative, negates a number to 2's complement. returns the number unsigned*/
int unsign(char param[]);
/*unsigns a number.If negative, negates a number to 2's complement returns the number unsigned*/
int unsignNum(int num);


/*createOutput.c*/
/*triggers the various output creation functions based on necessity. returns 0 upon error, 1 if succeeded.*/
int createOutput(char path[], labelNode *labelTableHead, short binary[], metadata *instructionLength, metadata *dataLength, externCallList *externCallListHead);
/*creates the ob file in base 64. returns 0 upon error, 1 if succeeded.*/
int printBase64(char path[], short binary[], int length, metadata *instructionLength, metadata *dataLength);
/*creates the ent file. returns 0 upon error, 1 if succeeded.*/
int printEnt(char path[], labelNode *labelTableHead);
/*creates the ext file. returns 0 upon error, 1 if succeeded.*/
int printExt(char path[], externCallList *externCallListHead);