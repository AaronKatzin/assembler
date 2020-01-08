#include "assembler.h"

/*prepares memory for and triggers loop of reading and processing params. Returns pointer to binary array or null in case of error*/
short *firstPass(char path[], labelNode *labelTableHead, dataNode *dataTableHead, short binary[], metadata *errorFlag){
	FILE *fp;

	char **params = calloc(1, sizeof(char));

	if(allocationCheck(1, params)){
		printf(ERROR"memory allocation failed during processing of %s, skipping file\n", path);
		return NULL;
	}

	
	/*sets to value that helps later determine if we are on the first node or not*/
	dataTableHead->address = EMPTY_LIST_ADDRESS;
	labelTableHead->address = EMPTY_LIST_ADDRESS;

	/*checks if file can be found and opened*/
	if((fp = fopen(path, "r")) == NULL){
		printf(ERROR"Can't open file, please make sure the file %s exists. moving to next file if relevant.\n", path);
		return NULL;
	}
	/*loops through the file*/
	binary = firstPassLoop(fp, params, binary, labelTableHead, dataTableHead, errorFlag, path);
	/*close the file*/
	fclose(fp);
	/*free params array*/
	free(params);

	return binary;
}

/*loop of reading and processing of input lines. Returns pointer to binary array or null in case of error*/
short *firstPassLoop(FILE *fp, char **params, short binary[], labelNode *labelTableHead, dataNode *dataTableHead, metadata *errorFlag, char path[]){
	int entry;
	int external;
	int numOrString;
	int labelFlag = 0;
	int DC = 0; /*data counter*/
	int IC = 0; /*instruction counter*/
	int dataLength;
	int opcode = -1; 
	int count = 0;
	int lineNumber = 0;
	int currentParam = 0;
	int numOfParams;
	int addressMethods[2];
	int length;

	while((numOfParams = readLine(fp, params, ++lineNumber))!= -1){/*until readline returns error code on EOF*/
		if(numOfParams < -1) {/*if readLine returned an error code*/
			errorFlag->value = 1;
			/*if there's an error, we don't need to do anything else on this line*/
		}
		else if(memoryCheck(IC, DC)){
			errorFlag->value = 1;
			printf(ERROR" Ran out of memory on line %d, aborting parsing\n", lineNumber);
			return NULL;
		}
		else if(lineNumber > MAX_LINES){
			errorFlag->value = 1;
			printf(ERROR" Too many lines, reached %d lines. Aborting parsing\n", lineNumber);
			return NULL;
		}
		else if(numOfParams == 0 || *params[0]== ';'){
			/*blank line or comment, do nothing*/
		}
		else{/*no quick errors, start parsing line*/
			if(label(params[0])){/*if first param is a label*/
				labelFlag = 1;
				
				if(params[currentParam+1]!=NULL){/*if there is another param on the line besides the label*/
					currentParam++;
				}
			}
			else{/*first param isn't label*/
				labelFlag = 0;
			}
			

			/*define flags*/
			numOrString = isData(params, numOfParams); 
			entry = isEntry(params, numOfParams);
			external = isExtern(params, numOfParams);
			
			
			if(labelFlag){/*deal with label*/
				/*1 if num, 2 if string, 0 if neither*/
				if(entry || external){
					printf("Warning: label declared for entry or extern on line %d, label not saved\n", lineNumber);
				}
				else if(numOrString){ /*data*/
					if(addTolabelTable(params[0], labelTableHead, DC, 0, external)){
						printf(ERROR"Can't add label on line %d\n", lineNumber);
						errorFlag->value = 1;
					}
				}
				else{/*instruction*/
					if(addTolabelTable(params[0], labelTableHead, IC, 1, external)){	
						printf(ERROR"Can't add label on line %d\n", lineNumber);
						errorFlag->value = 1;
					}
				}
				
			}
			if(external && !labelFlag){/*deal with extern*/
				if(!addExternalToLabelTable(params, labelTableHead, lineNumber)){/*check for error*/
					errorFlag->value = 1;
				}
			}

			else if(numOrString){/*deal with data*/				
				/*loops and gives all following params*/
				while(currentParam < numOfParams - 1){
					dataLength = addToDataTable(params[++currentParam], numOrString, DC, dataTableHead);

					/*checks if there was an issue adding operands to data table*/
					if (dataLength <= 0) {
						errorFlag->value = 1;
						printf(" on line %d\n", lineNumber);
					}
					
					/*adds number of data entries to DC counter*/
					DC += dataLength;
				}	
			}

			else if(entry){
				/*do nothing. deal with in second pass*/
			}
			
			else if((opcode = getActionOpcode(params[currentParam])) >= 0){/*deal with action*/
				/*zero the address method array, used to store address method number representation*/
				addressMethods[0] = 0;
				addressMethods[1] = 0;

				/*checks opernads and returns amount of words that will be needed to store them*/
				if((length = checkOperands(opcode, params, ++currentParam, numOfParams, addressMethods)) >= 0){/*if there are words to be added*/
					
					
					binary = (short*)realloc(binary, (IC + length+1)*sizeof(short));
					if(allocationCheck(1, params)){
						printf(ERROR"memory allocation failed during processing of %s, skipping file\n", path);
						return NULL;
					}
					binary = addCommandWord(opcode, addressMethods[0], addressMethods[1], binary, IC, length);/*add word*/
					if(allocationCheck(1, binary)){
						printf(ERROR"memory allocation failed during processing of %s, skipping file\n", path);
						return NULL;
					}
					IC+=length; /*add amount of words to IC counter*/
					
				}
				
				else{/*no words added*/
					errorFlag->value = 1;
					printf(" on line %d\n", lineNumber);
				}
				
			}
			
			else{/*didn't find action or data*/
				printf(ERROR"No action or data instruction found on line %d\n", lineNumber);
				errorFlag->value = 1;
			}
			
			/*free stored individual params*/
			for (count = 0; count < numOfParams; count++){
				free(params[count]);
			}
			/*zero the current param index*/
			currentParam = 0;
		}/*end else*/
		
	}/*end while*/

	/*update addresses after reading entire file and having proper addresses*/
	updateDataTableAddresses(dataTableHead, IC);
	updateLabelTableAddresses(labelTableHead, IC);

	return binary;
}