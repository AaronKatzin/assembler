#include "assembler.h"

/*checks if param is label. returns 1 if yes, 0 if not*/
int label(char param[]){
	int i;
	
	if(param != NULL && param[strlen(param)-1] == ':' && ((param[0] >= 'A' && param[0] <= 'Z') || (param[0] >= 'a' && param[0] <= 'z'))){/*checks if first letter is alphabetical*/
		for(i = 1; i < strlen(param) -1; i++){
			if(i > MAX_LABEL_LEN || (param[i] < 48 || (param[i] > 57 && param[0] < 'A') || (param[0] > 'Z' && param[0] < 'a') || (param[0] > 'z'))){/*checks if rest of letters are alphabetical or numerical*/
				return 0; /*not label*/
			}
		}
		return 1; /*label*/
	}
	return 0;/*not label*/
}

/*checks if params contain data or string keyword. data returns 1, string reurns 2 and neither returns 0*/
int isData(char *param[], int numOfParams){
	int index;
	for(index = 0; index < numOfParams; index++){/*iterate over params*/
		if(!strcmp(DATA, param[index])){/*check against data keyword*/
			return 1;/*found*/
		}
		else if(!strcmp(STRING, param[index])){/*check against string keyword*/
			return 2;/*found*/
		}
	}
	return 0;/*not found*/
}

/*checks if params contain entry keyword. 1 if true, 0 if false*/
int isEntry(char *param[], int numOfParams){
	int index;

	for(index = 0; index < numOfParams; index++){/*iterate over params*/
		if(!strcmp(param[index], ENTRY)){/*check against entry keyword*/
			return 1;/*found*/
		}
	}
	return 0;/*notfound*/
}

/*checks if params contain extern keyword. 1 if true, 0 if false*/
int isExtern(char *param[], int numOfParams){
	int index;

	for(index = 0; index < numOfParams; index++){/*iterate over params*/
		if(!strcmp(param[index], EXTERN)){/*check against extern keyword*/
			return 1;/*found*/
		}
	}
	return 0;/*not found*/
}

/*checks if param is absolute data. returns 1 if yes, 0 if not*/
int isAbsolute(char param[]){
	int i;
	
	/*check if first char is - or + or number*/
	if(param[0] != 43 && param[0] != 45 && ((param[0] < 48 || param[0] > 57))){
		return 0;
	}
	/*check if rest of chars are numbers*/
	for(i = 1; i < strlen(param) - 1; i++){
		if(param[i] < 48 && param[i] > 57){
			return 0;
		}
	}
	/*no issues found*/
	return 1;

}

/*checks if param is relocatable data. returns 1 if yes, 0 if not*/
int isRelocatable(char param[]){
	int i;

	/*make sure first char is lowercase or uppercase letter*/
	if((param[0] > 64 && param[0] < 91) || (param[0]>96 && param[0] < 123)){
		/*make sure each subsequent char is either a letter or number*/
		for(i = 1; i < strlen(param) -1; i++){
			
			if(i > MAX_LABEL_LEN || (param[i] < 48 || (param[i] > 57 && param[0] <65) || (param[0] > 90 && param[0] < 97)|| (param[0] > 122))){
				return 0; /*not label*/
			}
		}
		return 1; /*label*/
	}

	return 0; /*not label*/	
}

/*checks if param is a registry. returns 1 if yes, 0 if not*/
int isRegistry(char param[]){
	/*check if it's too long or short to be like @rX*/
	if (strlen(param) > 3){
		return 0;
	}
	/*checks if first char is @ */
	if(param[0] != '@'){
		return 0;
	}
	
	if(param[1] != 'r'){
		return 0;
	}
	/*checks if last char is number between 0 and 7*/
	if(param[2] < '0' || param[2] > MAX_REGISTRY_CHAR){
		printf(ERROR"Illegal Registry. Only registries between 0 and %d are allowed\n", MAX_REGISTRY);
		return 0;
	}
	return 1;

}

/*checks if destination method is legal for the given opcode. returns numerical value representing the method, or 0 if failed*/
int legalDestMethod(char param[], int opcode){
	if(opcode == 0 || (opcode >= 2 && opcode <= 11) || opcode == 13){
		if(isRelocatable(param)){
			return 3;/*notifies called that variable was sent*/
		}
		else if(isRegistry(param)){
			return 5; /*notifies caller that registry was sent*/
		}
		printf(ERROR " illegal destination method for action: %s", getActionName(opcode));
		return 0;   
		
	}
	else if(opcode == 1 || opcode || 12){
		if(isAbsolute(param)){
			return 1;/*notifies called that absolute was sent*/
		}
		else if(isRelocatable(param)){
			return 3;/*notifies called that variable was sent*/
		}
		else if(isRegistry(param)){
			return 5; /*notifies caller that registry was sent*/
		}
		printf(ERROR " destination source method for action: %s ", getActionName(opcode));
		return 0;
	}
	else if(opcode == 14 || opcode == 15){
		/*should never reach here because no operands are allowed*/
		printf(ERROR " destination method attempted even though no operands allowed for %s ", getActionName(opcode));
		return 0;

	}
	printf(ERROR " illegal action attempted, opcode %d\n", opcode);	
	return 0;
}

/*checks if souce method is legal for the given opcode. returns numerical value representing the method, or 0 if failed*/
int legalSourceMethod(char param[], int opcode){
	if((opcode > 3 && opcode < 6) || (opcode > 6 && opcode < 16)){
		printf(ERROR " source method checked even though no source allowed for action: %s", getActionName(opcode));
		return 0;
	}
	else if (opcode >= 0 && opcode <= 3) {
		if(isAbsolute(param)){
			return 1;
		}
		else if(isRelocatable(param)){
			return 3;
		}
		else if(isRegistry(param)){
			return 5; /*notifies caller that registry was sent*/
		}
		printf(ERROR "illegal source method for action: %s ", getActionName(opcode));
		return 0;
	}
	else if (opcode == 6) {
		if(isRelocatable(param)){
			return 3;
		}
		printf(ERROR "illegal source method for action: %s ", getActionName(opcode));
		return 0;
	}
	else{
		printf(ERROR " source method attempted even though no operand allowed for %s\n", getActionName(opcode));
		return 0;
	}
}

/*checks for too many quotes. reurns 1 if error if too many are found, 0 if not*/
int extraQuotes(char *line){
	int odd = 0; /*flag to be lit and off based on odd or even*/

	while (*line != 0){/*iterate over entire line*/
		if(*line == '"'){/*if quote found*/
			odd = !odd;/*change flag*/
		}
		line++;
	}
	return odd;
}

/*counts labels on the given line. skips labels in quotes. returns amount of lables found on line*/
int countLabel(char *line){
	int inQuotes = 0;/*in quote flag*/
	int label = 0;/*counter*/

	while(*line != 0){/*iterate over entire line*/
		if(*line == '"'){/*if quote found*/
			inQuotes = !inQuotes;/*change flag*/
		}
		if(!inQuotes && *line == ':'){/*finds label based on colons not inside quotes*/
			label++;
		}
		line++;
	}
	return label;
	
}

/*checks if given param is numerical. returns 1 if yes, 0 if no*/
int isNumeric(char *c){
	while(*c != 0){/*iterate over param char by char*/
		if((*c < 48 || *c > 57) && *c != '-' && *c != '+'){/*checks if it's not a number or sign*/
			return 0;/*not numerical*/
		}
		c++;
	}
	return 1;/*numerical*/
}

/*checks if given param is a reserved name. returns 1 if yes, 0 if no*/
int isReservedName(char *name){
	int index;
	const char *reserved[] = {RESERVED, ACTIONS}; /*array with reserved names and actions*/
	
	/*checks against reserved names*/
	for(index = 0; index < sizeof(reserved)/sizeof(char*); index++){
		if(!strcmp(name, reserved[index])){
			return 1;/*is reserved*/
		}
	}

	return 0; /*no match found, name not reserved*/
}

/*checks if there are any entries in the label table. returns 1 if yes, 0 if no*/
int existsEntry(labelNode *labelTableHead){
	labelNode *currentNode;
	for(currentNode = labelTableHead; currentNode != NULL; currentNode = currentNode->next){/*iterate over label table*/
		if(currentNode->isEntry){
			return 1;/*yes*/
		}
	}
	return 0;/*no*/
}

/*checks if there are any externs in the label table. returns 1 if yes, 0 if no*/
int existsExtern(labelNode *labelTableHead){
	labelNode *currentNode;
	for(currentNode = labelTableHead; currentNode != NULL; currentNode = currentNode->next){/*iterate over label table*/
		if(currentNode->isExtern){
			return 1;/*yes*/
		}
	}
	return 0;/*no*/
}

/*checks if given label is exernal. returns 1 if yes, 0 if no*/
int isLabelExtern(char label[], labelNode *labelTableHead){
    labelNode *currentNode;


    for(currentNode = labelTableHead; currentNode != NULL; currentNode = currentNode->next){/*find label in table*/
		if(!strcmp(currentNode->label, label)){
            return currentNode->isExtern;/*return value of flag*/
        }
	}
    return 0;/*not found, not external*/

}

/*checks if we overflowed the max mem limit based on instruction and data counters. returns 1 if yes, 0 if no*/
int memoryCheck(int IC, int DC){
	if(IC + DC + START_ADDRESS > MAX_MEM){
		return 1;
	}
	return 0;

}

/*checks if memory allocation returned null pointer. dynamic amount of params. first param is number of params, rest are pointers. returns 1 if yes, 0 if no*/
int allocationCheck(int num,...){
	va_list valist;
	int i; 

	va_start(valist, num);

	for(i=0; i < num; i++){/*iterate over params*/
		if(!va_arg(valist, void*)){/*if null*/
			va_end(valist);/*free list*/
			return 1;/*return yes*/
		}
	}
	va_end(valist);/*free list*/
	return 0;/*return no*/

}

/*checks if given char is space or tab. returns 1 if yes, 0 if no*/
int isWhiteSpace(char c){
	if(c == 9 || c == 32){
		return 1;
	}
	return 0;
}