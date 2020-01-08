#include "assembler.h"

/*Adds label to label table. returns 0 is ran succeeded, 1 if there was an error*/
int addTolabelTable(char param[], labelNode *labelTableHead, int address, int isCommand, int isExtern){
	char label[MAX_LABEL_LEN];
	labelNode *currentNode;

	/*check if label is too long. Uses max length + 1 to account for colon (:) */
	if(strlen(param)>MAX_LABEL_LEN+1){
		printf(ERROR"Label is too long\n");
		return 1;
	}

	/*copy and clean label string*/
	strcpy(label, param);

	if(label[strlen(label)-1] == ':'){/*in case it's a label, a colon (:) needs to be removed from the end. if .extern, no need to remove*/
		label[strlen(label)-1] = '\0';
	}

	
	/*checks if label is reserved assembly name*/
	if(isReservedName(label)){
		printf(ERROR"%s is a reserved name\n", label);
		return 1;
	}


	/*check if label already exists in table*/
	for(currentNode = labelTableHead; currentNode->next != NULL; currentNode = currentNode->next){
		if(!strcmp(currentNode->label, label)){
			printf(ERROR "label %s already exists in label table\n", label);
			return 1;
		}
	}
	if(!strcmp(currentNode->label, label)){/*need to check last element on list as well*/
		printf(ERROR "label %s already exists in label table\n", label);
		return 1;
	}

	
	if(labelTableHead->address == -1){/*add to head of table if table is empty*/
		strcpy(currentNode->label, label);
		if(isExtern){
			currentNode->address = address;
		}
		else
		{
			currentNode->address = address+START_ADDRESS;
		}
				
		currentNode->isCommand = isCommand;
		currentNode->isExtern = isExtern;
		currentNode->next = NULL;
	}
	else{/*add to end of table*/
		currentNode->next = (labelNode*)calloc(1, sizeof(labelNode)); /*allocate memory for node*/
		if(allocationCheck(1, currentNode->next)){/*memory issues check*/
			printf(ERROR"memory allocation failed during processing of param %s\n", param);
			return 1;
    	}
		currentNode = currentNode->next;
		strcpy(currentNode->label, label);
		if(isExtern){
			currentNode->address = address;
		}
		else
		{
			currentNode->address = address+START_ADDRESS;
		}
		currentNode->isCommand = isCommand;
		currentNode->isExtern = isExtern;
		currentNode->next = NULL;
	}
	
	/*no issues*/
	return 0;
}

/*adds data to data table. returns amount of words/elements added*/
int addToDataTable(char data[], int numOrString, int address, dataNode *dataTableHead){
	int size;
	char *token;
	dataNode *currentNode;


	for(currentNode = dataTableHead; currentNode->next != NULL; currentNode = currentNode->next){
		/*iterates pointer to end of list*/
	}
	/*deal with number*/
	if(numOrString == 1){
		token = strtok(data, DATA_DELIMS);
		for(size=0; token != NULL; size++){

			if(!isNumeric(token)){
				printf(ERROR "Data entered is not numerical even though .data keyword was used");
				return 0;
			}

			if(dataTableHead->address == -1){/*add to head of table if table is empty*/
				currentNode->value = atoi(token);
				currentNode->address = address+size;
				currentNode->numOrString = numOrString;
				token = strtok(NULL, DATA_DELIMS);
			}
			else{/*add to end of table*/
				currentNode->next = (dataNode*)calloc(1, sizeof(dataNode));/*allocate memory for node*/
				if(allocationCheck(1, currentNode->next)){/*memory issues check*/
					printf(ERROR"memory allocation failed during processing of data %s\n", data);
					return 1;
				}
				currentNode = currentNode->next;
				currentNode->value = atoi(token);
				currentNode->address = address+size;
				currentNode->numOrString = numOrString;
				token = strtok(NULL, DATA_DELIMS);
			}
		}
	}
	/*deal with string*/
	else if(numOrString == 2){
		/*make sure that string is surrounded by quotation marks*/
		if(data[0] != '\"' || data[strlen(data)-1] != '\"'){
			printf(ERROR "string not surrounded by quotation marks, data not stored");
			return 0;
		}
		/*remove quotation marks*/
		data = strtok(data, "\"");
		for(size = 0; size <= strlen(data); size++){/*add to head of table if table is empty*/
			if(currentNode->address == -1){
				currentNode->value = data[size];
				currentNode->address = address+size;
				currentNode->numOrString = numOrString;
				currentNode->next = NULL;
			}
			else{/*add to end of table*/
				currentNode->next = (dataNode*)calloc(1, sizeof(dataNode));
				if(allocationCheck(1, currentNode->next)){
					printf(ERROR"memory allocation failed during processing of data %s\n", data);
					return 1;
				}
				currentNode = currentNode->next;
				currentNode->value = data[size];
				currentNode->address = address+size;
				currentNode->numOrString = numOrString;
				currentNode->next = NULL;
			}
		}

	}
	/*returns number of elements added*/
	return size;
}

/*update data addresses after they are known. always returns 0.*/
int updateDataTableAddresses(dataNode *dataTableHead, int IC){
	dataNode *currentNode = dataTableHead;

	while(currentNode!=NULL){
		currentNode->address += IC;/*add instruction counter*/
		currentNode = currentNode->next;
	}
	return 0;

}

/*update label addresses after they are known. always returns 0.*/
int updateLabelTableAddresses(labelNode *labelTableHead, int IC){
	labelNode *currentNode = labelTableHead;

	while(currentNode!=NULL){
		if(!currentNode->isCommand && !currentNode->isExtern){
			currentNode->address += IC;/*add instruction counter*/
		}
		currentNode = currentNode->next;
	}
	return 0;
}

/*store external label on external list. always returns 1*/
int storeExt(char label[], int address, externCallList *externCallListHead){
	externCallList *currentNode;

	if(externCallListHead->address == -1){/*add to head of table if table is empty*/
		externCallListHead->address = address+START_ADDRESS;
		strcpy(externCallListHead->label, label);
		externCallListHead->next = NULL;
		return 1;
	}
	else{/*add to end of table*/
		for(currentNode = externCallListHead; currentNode->next != NULL; currentNode = currentNode->next){
			/*iterate to the end of the list*/
		}
		currentNode->next = (externCallList*)calloc(1, sizeof(externCallList));/*allocate memory for node*/
		if(allocationCheck(1, currentNode->next)){/*memory check*/
			printf(ERROR"memory allocation failed during processing of label %s\n", label);
			return 1;
		}
		currentNode = currentNode->next;
		currentNode->address = address+START_ADDRESS;
		strcpy(currentNode->label, label);
		currentNode->next = NULL;
	}
	return 1;
}

/*checks for duplicate labels. returns 1 if found. 0 if not.*/
int checkForDuplicateLabel(char *params[], labelNode *labelTableHead, int numOfParams){
	char *token;
	int index;

	/*loop through all params*/
	while(index < numOfParams){
		token = strtok(params[index], DATA_DELIMS);
		while(token != NULL){
			if(getLabelAddress(token, labelTableHead) != -1){/*get param's label address. if -1, means it doesn't exist*/
				return 1;/*found*/
			}
			token = strtok(NULL, DATA_DELIMS);/*load next param into buffer*/
		}
		index++;
	}
	/*not found*/
	return 0;
}

/*free the linked lists*/
void freeLists(externCallList *externCallListHead, labelNode *labelTableHead, dataNode *dataTableHead){
	/*define necessary pointers*/
	externCallList *tmpExt;
	externCallList *currentExtNode = externCallListHead;
	labelNode *tmpLabel;
	labelNode *currentLabelNode  = labelTableHead;
	dataNode *tmpData;
	dataNode *currentDataNode  = dataTableHead;

	/*free externals list*/
	while (currentExtNode != NULL){
		tmpExt = currentExtNode;
		currentExtNode = currentExtNode->next;
		free(tmpExt);
	}

	/*free label list*/
	while (currentLabelNode != NULL && strcmp(currentLabelNode->label, "")){
		tmpLabel = currentLabelNode;
		currentLabelNode = currentLabelNode->next;
		free(tmpLabel);
	}

	/*free data list*/
	while (currentDataNode != NULL){
		tmpData = currentDataNode;
		currentDataNode = currentDataNode->next;
		free(tmpData);
	}
}

/*adds externals to label table. returns 0 for errors, 1 for succcess*/
int addExternalToLabelTable(char **params, labelNode *labelTableHead, int lineNumber){
	char *token;
	int index = 0;

	token = strtok(params[1], " , .extern "); /*skips .index keyword*/
	
	/*adds label to label tabel with external flag*/
	if(addTolabelTable(token, labelTableHead, 0, 0, 1)){/*addToLabelTable returns positive if there's an error*/
		printf(ERROR"Can't add label on line %d\n", lineNumber);
		return 0;
	}
	else{/*label added successfully*/
		/*load potential next text into buffer*/
		token = strtok(NULL, " /t");
		index++;
	}
	
	if (token){/*checks if token isn't null. if not null, means there is more text.*/
		printf(ERROR"Extra text after external on line %d\n", lineNumber);
		return 0;
	}
	if(index == 0){/*if no externals were found*/
		printf(ERROR"external declared, but none found on line %d\n", lineNumber);
		return 0;
	}

	/*success*/
	return 1;
}