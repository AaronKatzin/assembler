#include "assembler.h"

/*gets th address of a label, returns -1 if no match found*/
int getLabelAddress(char label[], labelNode *labelTableHead){
    labelNode *currentNode;

    /*iterates through label tabel until it finds a match*/
    for(currentNode = labelTableHead; currentNode != NULL; currentNode = currentNode->next){
		if(!strcmp(currentNode->label, label)){/*match found*/
            /*return address*/
            return currentNode->address;
        }
	}
    /*returns -1 if no match found*/
    return -1;

}

/*returns the base 64 char that represents the given value*/
char getBase64(short bin){
    const char b64_table[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/'
    };
    return b64_table[bin];
}

/*sets entry labels with entry flag. retuns 0 for errors and number of labels marked if successful*/
int setEntry(char *params[], int currentParam,  labelNode *labelTableHead){
	char *token;
	labelNode *currentNode;
	int labelCount = 0; /*labels successfully found*/
	int trialCount = 0; /*labels attempted*/

    /*skip the entry param*/
	if(!strcmp(params[currentParam], ENTRY)){
		currentParam++;
	}

    /*load next label into token buffer*/
	token = strtok(params[currentParam], " ,");
	trialCount++;

    /*iterate over whole label table, looking for a match to the current buffer token*/
	for(currentNode = labelTableHead; currentNode != NULL; currentNode = currentNode->next){
		if(!strcmp(token, currentNode->label)){/*match found*/
			currentNode->isEntry = 1;/*set entry flag*/
			labelCount++;
		}
	}

    /*continues loading text into buffer. shouldn't be any left so throws error*/
	if((token = strtok(NULL, ", \t"))!=NULL){
		printf(ERROR"Extra text after entry found");
		return 0;
	}

    /*compares the amount of labels attempted to flag as entry with actual number flagged*/
	if(trialCount>labelCount){
		printf(ERROR"Entry label expected, but not found");
		return 0;
	}

	return labelCount;
}

/*returns the number of operands permitted for an action. return -1 if illegal opcode is passed*/
int getPermittedOpCount(int opcode){
	if((opcode >= 0 && opcode <= 3) || opcode == 6){
		return 2;
	}
	else if((opcode >= 7 && opcode <= 13) || opcode == 4 || opcode == 5){
		return 1;
	}
	else if(opcode == 14 || opcode == 15){
		return 0;
	}
	printf(ERROR"Illegal opcode %d\n", opcode);
	return -1;
}

/*returns a string with the name of the action based on the opcode given. null if there was an error*/
char *getActionName(int opcode){
	char *actionName = calloc(4, sizeof(char));/*prepare string for name to be stored*/
	const char *actions[] = {ACTIONS};	/*array of actions*/

	if(allocationCheck(1, actionName)){/*check for memory issue */
		printf(ERROR"memory allocation failed during processing \n");
		return NULL;
	}
	if(opcode < 0 || opcode > MAX_OPCODE){/*check for out of bounds*/
		return NULL;
	}

	/*copy from array to name string*/
	strcpy(actionName, actions[opcode]);

	return actionName;
}

/*gets opcode of action based on string. returns -1 if not found.*/
int getActionOpcode(char param[]){
	int i;
	const char *actions[] = {ACTIONS};/*array of actions*/
	
	for(i = 0; i < sizeof(actions)/sizeof(char*); i++){
		if(!strcmp(param, actions[i])){
			return i;
		}
	}
	/*none found*/
	return -1;
}

/*sets bits based on the shift and value given. retuns word with bits set*/
short setBits(int shift, int value){
    short word = 0;

    value <<= shift;
    word |= value;

    return word;
}

/*gets bits based on shift, length of the mask needed, and the given word that the bits are taken from*/
short getBits(int shift, int length, short word){
    short mask = getMask(length);
    
    word >>= shift;

    return word & mask;
}

/*returns the mask needed for the length given*/
short getMask(int length){
    short mask = 0;
    const int binaryOffset[] = {BINARY_OFFSET};

    /*makes sure that it's in bounds, and then gives the value. uses -1 because value of length one is stored at index 0 on the binary offset array*/
    if (length<= WORD && length >= 1){
        mask = binaryOffset[length-1];
    }
    /*returns value. 0 if length was not legitimate or 0*/
    return mask;
}

