#include "assembler.h"

/*adds command word to binary array. returns pointer to updated binary array or null if there was an error*/
short* addCommandWord(int opcode, int sourceMethod, int destMethod, short *binary, int IC, int length){
    /*allocate memory for additional word*/
    binary = (short*)realloc(binary, (IC + length+1)*sizeof(short)); 
    if(allocationCheck(1, binary)){
        return NULL;
	}

    /*ensure that word starts as zero*/
    binary[IC] = 0;

    /*deal with bits 2-4 (destination method)*/
    binary[IC] |= setBits(2, destMethod);

    /*deal with bits 5-8 (opcode)*/
    binary[IC] |= setBits(5, opcode);

    /*deal with bits 9-11 (source method)*/
    binary[IC] |= setBits(9, sourceMethod);

    return binary;
}

/*add operand words to binary array. returns amount of words added*/
int addOperandWord(char param[], int IC, short binary[], labelNode *labelTableHead, externCallList *externCallListHead){
    int sourceMethod = 0, destMethod = 0;
    char *token;
    /*get operand types based on action word's values*/
    sourceMethod |= getBits(9, 3, binary[IC-1]);
    destMethod |= getBits(2, 3, binary[IC-1]);
    if(!sourceMethod && !destMethod){/*no operands expected*/
        printf(ERROR"No operands expected \n");
        return 0;
    }
    /*deals with case of only one operand*/
    else if(!sourceMethod){
        if(destMethod == REGISTRY){/*registry*/
            /*take char from x place in string,converts it to int and place in word*/
            /*the loaction of the values can be assumed because the row and string were checked and cleaned earlier*/
            binary[IC] |= setBits(7, param[2] - 48);

            return 1;
        }
        else if(destMethod == RELOCATABLE){/*label*/
            /*check if label exists*/
            if(getLabelAddress(param, labelTableHead) == -1){
                return 0;
            }
            binary[IC] |= setBits(2, getLabelAddress(param, labelTableHead));
            if(isLabelExtern(param, labelTableHead)){
                binary[IC] |= setBits(0, 1); /*0b01 for external*/
                storeExt(param, IC, externCallListHead);
            }
            else{
                binary[IC] |= setBits(0, 2); /*0b10 for relocatable*/
            }
            return 1;

        }
        else if(destMethod == ABSOLUTE){/*absolute*/
            /*no need to touch bits 0 and 1, already 0.
            Store in bits 2-6*/
            binary[IC] |= setBits(2, unsign(param));

            return 1;

        }
        return 0; /*error*/  
    }
    /*deals with case of two operands*/
    else if(sourceMethod && destMethod){

        /*deal with two registries in one word]*/
        if(sourceMethod == REGISTRY && destMethod == REGISTRY){
            /*dest value stored in bits 2-6*/
            /*source value stored in bits 7-11*/
            binary[IC] |= setBits(7,  param[2] - 48);
            binary[IC] |= setBits(2,  param[6] - 48);
            return 1;
        }
        token = strtok(param, ",");

        /*deal with source*/
        if(sourceMethod == ABSOLUTE){
            binary[IC++] |= setBits(2, unsign(token)); /*increase IC for use in destination param*/
            token = strtok(NULL, ","); /*load next operand to buffer*/
        }
        else if(sourceMethod == RELOCATABLE){/*label*/
            /*check if label exists*/
            if(getLabelAddress(token, labelTableHead) == -1){
                printf(ERROR"label or variable %s not found\n", token);
                return 0;
            }
            binary[IC] |= setBits(2, getLabelAddress(token, labelTableHead));
            if(isLabelExtern(token, labelTableHead)){
                binary[IC] |= setBits(0, 1); /*0b01 for external*/
                storeExt(token, IC++, externCallListHead);
            }
            else{
                binary[IC++] |= setBits(0, 2); /*0b10 for relocatable*/
            }
            token = strtok(NULL, ","); /*load next operand to buffer*/
        }
        else if(sourceMethod == REGISTRY){ 
            binary[IC++] |= setBits(7, token[2]-48); /*increase IC for use in destination param*/
            token = strtok(NULL, ","); /*load next operand to buffer*/
        }

        /*deal with dest*/
        if(destMethod == ABSOLUTE){
            binary[IC] |= setBits(2, unsign(token)); 
            return 2;
        }
        else if(destMethod == RELOCATABLE){
            /*check if label exists*/
            if(getLabelAddress(token, labelTableHead) == -1){
                return 0;
            }
            binary[IC] |= setBits(2, getLabelAddress(token, labelTableHead));
            if(isLabelExtern(token, labelTableHead)){
                binary[IC] |= setBits(0, 1); /*0b01 for external*/
                storeExt(token, IC, externCallListHead);
            }
            else{
                binary[IC] |= setBits(0, 2); /*0b10 for relocatable*/
            }
            return 2;
        }
        
        else if(destMethod == REGISTRY){
            binary[IC] |= setBits(2, token[2]-48);
            return 2;
        }
        
    }
    /*no words added*/
    return 0;
}

/*adds data table to binary array of shorts. returns number ofdata elements added*/
int addDataToBinary(dataNode *dataTableHead, short binary[]){
    /*pointer to be used as index*/
    dataNode *currentNode;
    /*count of how many elemnts added, to be returned */
    int count;

    /*for loop on entire data table*/
    for(currentNode = dataTableHead, count = 0; currentNode != NULL; currentNode = currentNode->next, count++){
        /*set element at the node's address value to the node's value*/
        binary[currentNode->address] = unsignNum(currentNode->value);
	}
    return count;
}

/*unsigns a number from a character array. If positive, drops the plus sign. If negative, negates a number to 2's complement. returns the number unsigned*/
int unsign(char param[]){
    if(param[0] == '-'){/*if negative*/
        return (~atoi(++param)) + 1; /*ignore '-' sign, take flipped number, and add one*/
    }
    /*if positive or unsigned*/
    return atoi(param);
}

/*unsigns a number.If negative, negates a number to 2's complement returns the number unsigned*/
int unsignNum(int num){
    if (num <= 0){
        return ~(-1*num) + 1;
    }
    return num;
}


