#include "assembler.h"

/*triggers the various output creation functions based on necessity. returns 0 upon error, 1 if succeeded.*/
int createOutput(char path[], labelNode *labelTableHead, short binary[], metadata *instructionLength, metadata *dataLength, externCallList *externCallListHead){
    /*create ob file*/
    if(!printBase64(path, binary, instructionLength->value + dataLength->value, instructionLength, dataLength)){/*check for errors*/
        printf(ERROR"Can't create ob file\n");
        return 0;
    }
    /*create ent file if necessary*/
    if(existsEntry(labelTableHead)){
        if(!printEnt(path, labelTableHead)){/*check for errors*/
            printf(ERROR"Can't create ent file\n");
            return 0;
        }
    }
    else{/*notify user that no entry file was saved*/
        printf("No entry locations detected and therefore entry location file not saved.\n");
    }
    /*create ext file if necessary*/
    if(existsExtern(labelTableHead)){
        if(!printExt(path, externCallListHead)){/*check for errors*/
            printf(ERROR"Can't create ext file\n");
            return 0;
        }
    }
    else{/*notify user that no extern file was saved*/
        printf("No external references detected and therefore external reference file not saved.\n");
    }

    /*no errors*/
    return 1;
}

/*creates the ob file in base 64. returns 0 upon error, 1 if succeeded.*/
int printBase64(char path[], short binary[], int length, metadata *instructionLength, metadata *dataLength){
    int index = 0;
    short left;
    short right;
    FILE *fp;
    char *ob;

    /*allocate memory for path to be stored*/
    ob = calloc((strlen(path)+3)/sizeof(char), sizeof(char));
    if(allocationCheck(1, ob)){
        printf(ERROR"memory allocation failed during processing of %s, skipping file\n", path);
        return 0;
    }

    /*copy path*/
    strcpy(ob, path);
    /*add ent to name*/
    strcat(ob, ".ob");

    if(!(fp = fopen(ob, "w"))){/*return error if fopen fails*/
        return 0;
    }

    /*print lengths*/
    fprintf(fp, "%d %d\n", instructionLength->value, dataLength->value);

    /*Convert binary to base64 and print*/
    while(index < length){
        left = 0;
        right = 0;
        left |= getBits(6, 6, binary[index]);
        right |= getBits(0, 6, binary[index++]);
        fprintf(fp, "%c%c\n", getBase64(left), getBase64(right));
    }

    /*notify user that file was saved*/
    printf("Assembled data stored in %s\n", ob);

    /*close and free*/
    fclose(fp);
    free(ob);

    /*no errors*/
    return 1;
}

/*creates the ent file. returns 0 upon error, 1 if succeeded.*/
int printEnt(char path[], labelNode *labelTableHead){
    char *ent;
    FILE *fp;
    labelNode *currentNode;

    /*allocate memory for path to be stored*/
    ent = calloc((strlen(path)+4)/sizeof(char), sizeof(char));
    if(allocationCheck(1, ent)){
        printf(ERROR"memory allocation failed during processing of %s, skipping file\n", path);
        return 0;
    }
    /*copy path*/
    strcpy(ent, path);
    /*add ent to name*/
    strcat(ent, ".ent");


    if(!(fp = fopen(ent, "w"))){/*return error if fopen fails*/
        return 0;
    }

    /*loop entire label list and print entry labels and addresses to file*/
    for(currentNode = labelTableHead; currentNode != NULL; currentNode = currentNode->next){
		if(currentNode->isEntry){
            fprintf(fp, "%s\t%d\n", currentNode->label, currentNode->address);
        }
	}   

    /*notify user that file was saved*/
    printf("Entry locations stored in %s\n", ent);
    
    /*close and free*/
    fclose(fp);
    free(ent);

    /*no errors*/
    return 1;
    
}

/*creates the ext file. returns 0 upon error, 1 if succeeded.*/
int printExt(char path[], externCallList *externCallListHead){
    char *ext;
    FILE *fp;
    externCallList *currentNode;

    /*allocate memory for path to be stored*/
    ext = calloc((strlen(path)+4)/sizeof(char), sizeof(char));
    if(allocationCheck(1, ext)){
        printf(ERROR"memory allocation failed during processing of %s, skipping file\n", path);
        return 0;
    }

    /*copy path*/
    strcpy(ext, path);
    /*add ext to name*/
    strcat(ext, ".ext");

    if(!(fp = fopen(ext, "w"))){/*return error if fopen fails*/
        return 0;
    }

    /*loop entire external call list and print labels and addresses to file*/
    for(currentNode = externCallListHead; currentNode != NULL; currentNode = currentNode->next){
        fprintf(fp, "%s\t%d\n", currentNode->label, currentNode->address);
	}
    
     /*notify user that file was saved*/
    printf("Locations referencing externs stored in %s\n", ext);

    /*close and free*/
    fclose(fp);
    free(ext);

    /*no errors*/
    return 1;
    
}