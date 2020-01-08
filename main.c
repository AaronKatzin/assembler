#include "assembler.h"

/*main function accepts command line arguments and uses them via argv. Always returns 1.*/
int main(int argc, char* argv[]){
	int arg;							/*argument index*/
	short *binary;						/*pointer to binary array*/
	char *path;							/*pointer to string that holds the path of the file*/
	labelNode *labelTableHead; 			/*pointer to head of label table*/
	dataNode *dataTableHead;			/*pointer to head of data table*/
	externCallList *externCallListHead;	/*pointer to head of externals table*/
	metadata *instructionLength;		/*pointer to struct that holds numerical value for instruction length*/
	metadata *dataLength;				/*pointer to struct that holds numerical value for data length*/
	metadata *errorFlag;				/*pointer to struct that holds numerical value for errorflag*/
	
	if(argc == 1){/*deal with case that no arguments given*/
		printf(ERROR"No file specified. Please run program with filename(s) specified\n");
		return 1;
	}

	/*for loop to iterate over each argument/file name passed*/
	for(arg = 1; arg < argc; arg++){

		/*User prompt*/
		printf("Assembling %s\n", argv[arg]);
		
		/*allocation of memory*/
		labelTableHead = (labelNode*)calloc(1, sizeof(labelNode));
		dataTableHead = (dataNode*)calloc(1, sizeof(dataNode));
		binary = (short*) calloc(1, sizeof(short));
		instructionLength = malloc(sizeof(metadata));
		externCallListHead = malloc(sizeof(externCallList));
		dataLength = malloc(sizeof(metadata));
		errorFlag = malloc(sizeof(metadata));

		/*check for memory allocation issues*/
		if(allocationCheck(7, labelTableHead, dataTableHead, binary, instructionLength, externCallListHead, dataLength, errorFlag)){
			printf(ERROR"memory allocation failed during processing of %s, skipping file\n", argv[arg]);
			continue;
		}

		/*zero values*/
		instructionLength->value = 0;
		dataLength->value = 0;
		errorFlag->value = 0;

		/*prepare path*/
		path = calloc((strlen(argv[arg])+3)/sizeof(char), sizeof(char));
		if(allocationCheck(1, path)){/*check for memory allocation issues*/
			printf(ERROR"memory allocation failed during processing of %s, skipping file\n", argv[arg]);
			continue;
		}
		strcpy(path, argv[arg]);
		strcat(path, ".as");
		
		/*trigger first pass, and catch pointer to binary that is returned. coud be null, in which case we deal with error immediately*/
		binary = firstPass(path, labelTableHead, dataTableHead, binary, errorFlag);
		if(allocationCheck(1, binary)){/*check for memory allocation issues*/
			continue;/*skip this file*/
		}

		/*check if there were errors during first pass*/
		if(errorFlag->value){
			printf("Due to errors, output files not created for %s\n", argv[arg]);
		}
		else if(!secondPass(path, labelTableHead, dataTableHead, binary, instructionLength, dataLength, externCallListHead)){/*check if there were errors during second pass*/
			printf("Due to errors, output files not created for %s\n", argv[arg]);
		}
		else{/*no errors during passes. data is ready, so create output*/
			createOutput(argv[arg], labelTableHead, binary, instructionLength, dataLength, externCallListHead);
		}

		/*free memory*/
		free(binary);
		free(instructionLength);
		free(path);
		free(dataLength);
		free(errorFlag);
		freeLists(externCallListHead, labelTableHead, dataTableHead);

		/*User prompt*/
		printf("Finished assembling %s\n", argv[arg]);

	}
	printf("Assembler completed. Exiting.\n");
	return 1;
}
