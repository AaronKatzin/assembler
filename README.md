
Situations I know my assembler can't handle:

	Can't handle strings of data with spaces. I noticed this very soon before the time we had to submit the project. 
	Unfortunately I didn't have time to fix it.
	
	
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

