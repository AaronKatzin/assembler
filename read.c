#include "assembler.h"

/*reads line from buffer, clean, split into separate params. returns number of params found on line. negative number represents error*/
int readLine(FILE *fp, char *params[], int lineNumber){
	char rawLine[MAX_IN];
	char line[MAX_IN];
	int numOfParams;

	if(!fgets(rawLine, MAX_IN, fp)){/*load raw line into buffer and check for errors on fgets*/
		return -1;
	}	
	cleanLine(rawLine, line);/*cleans raw line. places cleaned line into 'line'*/


	line[strcspn(line, "\n")] = 0;/*gets rid of newline char placed by fgets*/

	if(extraCommas(line)){/*check for extra commas*/
		printf(" on line %d\n", lineNumber);
		return -2;
	}
	if(extraQuotes(line)){/*check for extra quotes*/
		printf(ERROR "Odd amount of many quotes on line, can't read string %d\n", lineNumber);
		return -3;
	}
	if(countLabel(line)>1){/*check for extra labels*/
		printf(ERROR "Too many colons (:) on line %d, only one label per line is allowed\n", lineNumber);
		return -3;
	}

	numOfParams = separateParams(line, params);/*separate params. catch return from function*/

	return numOfParams;
}

/*separates line into params. returns number of params. negative for error*/
int separateParams(char *line, char *params[]){
	char *token = strtok(line, " \t ");
	int j = 0;
	int numOfParams = 0;

	
	for(j=0;  token != NULL && j < MAX_SUBSTRINGS; j++){/*loop until token strtok returns null*/
		(params)[j] = calloc(MAX_PARAM_LEN, sizeof(char));/*allocate memory for param*/
		if(allocationCheck(1, (params)[j])){/*check for memory error*/
			printf(ERROR"memory allocation failed during processing \n");
			return -1;/*error*/
		}
		strcpy(params[j], token);/*copy param into newly allocated memory*/
		token = strtok(NULL, " :\t "); /*load next param into buffer*/
		numOfParams++;

	}
	return (numOfParams>=0)? numOfParams /*good number*/: -1;/*error*/
	

}

/*checks if operands are legitimate. returns number of words that will be needed, including command word. -1 for error*/
int checkOperands(int opcode, char *params[], int currentParam, int numOfParams, int addressMethods[]){
	char firstOp[MAX_LABEL_LEN];
	char secondOp[MAX_LABEL_LEN];
	int count;
	int permittedOpCount;

	char *token = strtok(params[currentParam], ",");/*load first operand into buffer*/

	permittedOpCount = getPermittedOpCount(opcode); /*find how many operands allowed based on opcount*/
	if(permittedOpCount == -1){
		return -1; /*illegal opcode*/
	} 

	if(currentParam > numOfParams -1){ /*no params after action*/
		if(permittedOpCount == 0){
			return 1; /*no operands necessary, return 1 for instruction word*/
		}
		printf(ERROR"No Operands detected, but operand(s) necessary for action %s ", getActionName(opcode));
		return -1;
	}
	
	for(count = 1; token != NULL && count <= permittedOpCount + 1;){/*iterate over rest of buffer*/
		if(count > permittedOpCount){
			printf(ERROR "too many operands for action %s ", getActionName(opcode));
			return -1;
		}
		if(count == 1){
			strcpy(firstOp, token);/*copy first operand*/
			token = strtok(NULL, ",");
			if(token){
				count++;
			}
			
		}
		else if(count == 2){ 
			strcpy(secondOp, token);/*copy second operand*/
			token = strtok(NULL, ",");
			if(token){
				count++;
			}
		}
	}
	
	if(count != permittedOpCount){/*check for wrong number of operands*/
		printf(ERROR "wrong number of operands for action %s ", getActionName(opcode));
		return -1;
	}

	if(count == 0){
		return 1; /*only one word needed, for the instruction*/
	}

	else if(count == 1){
		
		if(legalDestMethod(firstOp, opcode)){/*check if method is an allowed method*/
			addressMethods[1] = legalDestMethod(firstOp, opcode);/*place method into array for later use*/
			addressMethods[0] =0;
			return 2; /*one word for instruction, another for single data*/
		}
		else{
			return -1;/*illegal method*/
		}
	}
	else if(count ==2){
		addressMethods[1] = legalDestMethod(secondOp, opcode);/*check if dest method is an allowed method*/
		addressMethods[0] = legalSourceMethod(firstOp, opcode);/*check if source method is an allowed method*/
		if(addressMethods[1] == 5 && addressMethods[0] == 5){
			addressMethods[1] = 5;/*place methods into array for later use*/
			addressMethods[0] = 5; 
			return 2; /*Both are registries so: one word for instruction, another word for 2 registries to share a word*/
		}
		else if(addressMethods[1] && addressMethods[0]){
			return 3; /*Both are legal but both aren't registries, so: one word for instruction, another 2 words for data*/
		}
		else{
			return -1; /*one of the operands is illegal*/
		}
	}
	printf(ERROR " check operands failed\n");
	return -1;
	
}

/*checks for extra commas on given line. returns 1 if extra comma found, 0 if not*/
int extraCommas(char line[]){
	int i;
	int commaFlag = 0;
	int colonFlag = 0;

	/*ignore if comment*/
	if(line[0]!= ';'){
		for(i = 0; i < strlen(line); i++){/*iterate over line*/
			if(line[i] == 9 || line[i] == 32){
				/*space or tab, do nothing*/
			}
			else if(!commaFlag){/*if not after comma*/
				if(line[i] == 44){/*if current char is comma*/
					commaFlag = 1;/*light flag*/
				}
				if(line[i] == ':'){/*if current char is colon*/
					colonFlag = 1;/*light flag*/
				}
			}
			else if(commaFlag || colonFlag){/*if after comma*/
				if(line[i] == 44){/*if current char is comma*/
					/*extra comma*/
					printf(ERROR "Too many commas detected ");
					return 1;
				}
				else{
					/*lower the flags*/
					commaFlag = 0;
					colonFlag = 0;
				}
			}
		}
		if(commaFlag){
			printf(ERROR "Extra comma at the end of the line detected ");
			return 1;
		}
	
	}
	return 0;/*legitimate*/
}

/*cleans raw line from whitespaces*/
void cleanLine(char *src, char *dest){
    int isInsideQuotation = 0;
    int isAfterComma = 0;
	int isAfterColon = 0;
	
	while (isWhiteSpace(*src)){/*skips whitespaces*/
        src++;
    }

    while (*src != 0){/*loop over line*/
		if(*src == ':'){
			isAfterColon = 1;
		}
        if (*src == '"'){
            isInsideQuotation = !isInsideQuotation; /* flip flag based on quotes.*/
            isAfterComma = 0;
        }
        if (isInsideQuotation || isAfterColon){/*if inside quote or is immediately after colon, copies unconditionally*/
            *dest = *src;
            dest++;

			if (isAfterColon){/*lower colon flag*/
				isAfterColon = !isAfterColon;
			}
        }
        else if (!isWhiteSpace(*src)){/*not a white space*/
            *dest = *src;/*copy*/
            dest++;/*iterate*/

            if (isAfterComma){/*lower comma flag*/
                isAfterComma = 0;
            }
            if (*src == ','){/*raise comma flag*/
                isAfterComma = 1;
            }
        }
        else if (!isAfterComma && !isWhiteSpace(*(src+1)) && *(src+1) != ',' && *(src+1) != '\n' && *(src+1) != '\0'){/*copy space but make sure its not end of line space*/
            *dest = ' ';
            dest++;
        }
        src++;
    }
    *dest = '\0';/*newline character*/
}