#include "assembler.h"

/*prepares memory for and triggers loop of reading and processing params. Returns 0 in case of success and 1 in case of error*/
int secondPass(char path[], labelNode *labelTableHead, dataNode *dataTableHead, short binary[], metadata *instructionLength, metadata *dataLength, externCallList *externCallListHead){
    char **params = calloc(1, sizeof(char)); /*array of pointers to parameter strings*/
    FILE *fp;
    int errorFlag = 0;	

    if(allocationCheck(1, params)){
        printf(ERROR"memory allocation failed during processing of %s, skipping file\n", path);
        return 1;
    }

    externCallListHead->address = EMPTY_LIST_ADDRESS; /* -1 symbolizes first node of list*/

    fp = fopen(path, "r");
    /*loops through the file*/
    if(secondPassLoop(params, fp, labelTableHead, dataTableHead, binary, instructionLength, dataLength, externCallListHead)){
        errorFlag = 1;
    }
    /*free params array*/
    free(params);
    /*close file*/
    fclose(fp);
  
    return !errorFlag; /*return errorflag to main, to determine if to continue to create output files*/
}

/*loop of reading and processing of input lines.  Returns 0 in case of success and 1 in case of error*/
int secondPassLoop(char **params,FILE *fp, labelNode *labelTableHead, dataNode *dataTableHead, short binary[], metadata *instructionLength, metadata *dataLength, externCallList *externCallListHead){
    int IC = 0;
    int numOfParams;
    int errorFlag = 0;
    int lineNumber = 0;
    int numOrString;
    int currentParam = 0;
    int entry;
	int external;
    int count;
    int prevIC;
    
    while((numOfParams = readLine(fp, params, ++lineNumber))!= -1){/*until readline returns error code on EOF*/
        
        /*define flags*/
        numOrString = isData(params, numOfParams); 
		entry = isEntry(params, numOfParams);
		external = isExtern(params, numOfParams);
        currentParam = 0;

        if(numOfParams < -1){/*if readLine returned an error code*/
			errorFlag = 1;
			/*if there's an error, we don't need to do anything else on this line*/
		}
        else if(numOfParams == 0 || *params[0]== ';'){
			/*blank line or comment, do nothing*/
		}
        else if(numOrString){
            /*data, already dealt with in first pass*/
        }
        else if(external){
            /*do nothing at this point*/
        }
        else{
            if(label(params[0])){
                currentParam++; /*skip it, already dealt with in first pass*/
            }
            if(entry){/*deal with .entry*/
                if(!setEntry(params, currentParam, labelTableHead)){/*set entry using function, check for errors*/
                    printf(" on line %d\n", lineNumber);
                    errorFlag = 1; 
                }
                currentParam++;
            }
            else if(getActionOpcode(params[currentParam]) == 14 || getActionOpcode(params[currentParam]) == 15){
                /*no operands to deal with */
                IC++;/*raise IC for one action word with no params/data following*/

            }
            else if(getActionOpcode(params[currentParam]) != -1){/*deal with action*/
                IC++;/*add to IC for one instruction word*/
                currentParam++; /*iterate current param*/
                prevIC = IC;/*previous IC stored to later compare*/
                IC += addOperandWord(params[currentParam], IC, binary, labelTableHead, externCallListHead); /*call add word function*/
                if(IC == prevIC){/*no wordds added, so we attempted to add operand but faild*/
                    printf(ERROR"operand processing on line %d failed\n", lineNumber);
                    errorFlag = 1;
                }
            } 
        }
        /*free stored individual params*/
        for (count = 0; count < numOfParams; count++) {
            free(params[count]);
		}
    }/*end while loop*/

       

    instructionLength->value = IC; /*store instruction length to be passed on to 'create output'*/

    IC += addDataToBinary(dataTableHead, binary); /*calculate data length*/
    dataLength->value =  IC - instructionLength->value;/*store data length to be passed on to 'create output'*/

    return errorFlag;
}