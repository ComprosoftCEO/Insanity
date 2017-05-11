#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> 
#include <unistd.h>
#include <time.h>

//Platform independent version of Insanity!


//          $$$$$$$$$$$$$$$
//          $ Definitions $
//          $$$$$$$$$$$$$$$
   
typedef unsigned short int uint16;
typedef unsigned char byte;
typedef unsigned int uint32;
typedef unsigned int uint;

#define false 0
#define true 1




//          $$$$$$$$$$$$$$$$
//          $ All Commands $
//          $$$$$$$$$$$$$$$$

/*  0  = Stop program .

  |Jumping and Subroutines|
    1  = Jump ()
    2  = Jump Subroutine []
    3  = Return from subroutine ;

  |Cursors|
    4  = Memory cursor right >
    5  = Memory cursor left <
    6  = Increase digit counter by 10 "
    7  = Decrease digit counter by 10 '
    8  = Reset memory cursor and digit counter _
  
  |Memory and Register|
    9  = Upload memory to acc ^
    10 = Swp memory and acc |
    11 = Save acc to backup $
    12 = Swo acc anc backup ~

  |Maths|
    13 = Add +
    14 = Subtract - 
    15 = Add backup &
    16 = Multiply acc by -1  `
    17 = Reset acc to 0   @
    18 = Set acc to random number (-999 to 999)  %

  |Comparisons|
    19 = EQU  =
    20 = NEQ  *
    21 = GTR  /
    22 = LSS  \
    23 = Overflow  !
    24 = Jump if compare true {}

  |User Input|
    25 = Ask for user input & store in acc (-999 to 999)   ? 
    26 = Output acc as text char #
    27 = Pause the program , [Also shows debug information]
*/

//All commands listed in order
static const char* commands = ".([;><\"'_^|$~+-&`@%=*/\\!{?#,";




//          $$$$$$$$$$$$$$$$$$$$
//          $ Global Variables $
//          $$$$$$$$$$$$$$$$$$$$


//-----Registers/Flags/Cursors-----
typedef struct {
	int acc : 11;		//Accumulator
	int bak : 11;		//Backup

	uint32 pc;		//Program counter
	uint sp : 7;		//Stack pointer
	uint stackProb : 2;	//Flag for any stack problems (1 = Overflow, 2 = Underflow)

	short int memory[1000];	//1000 memory slots
	uint32 stack[100];	//Store 100 stack values (Subroutines)
	byte* program;		//Program memory

	uint overflow : 1;	//Overflow flag
	uint compare : 1;	//Compare flag
	uint mc : 10;		//Memory cursor
	uint dig : 2;		//Digit cursor
  } REG;

//--------Store a label--------
typedef struct {
    char* name;		//What is the textual name of this label
    uint32 address;	//What is the address of this label
    byte used;		//Was this label used???
} label;


REG sys;	// Bitfield used to save space and be AWESOME!!!

uint32 curByte, maxByte;	//Used when parsing file
uint lblCount, resolveCount;	//Used to keep track of labels




//          $$$$$$$$$$$$$$$$$$$$$$$
//          $ Function Prototypes $
//          $$$$$$$$$$$$$$$$$$$$$$$


//Main Functions
void reset();
void drawDebug(label*, byte*);
void pauseProgram();
void debugPause();

//Binary Functions
void storeWord(byte*, uint32); 
uint32 getWord(byte*);

//File Parsing Functions
byte parseFile(FILE*); 
void parseFail();
void expandProgram(byte**, byte);

//Label Functions
byte charToCommand(char);
char* readLabel(FILE*, char);
byte duplicateLabel(char*,label*);
uint32 findLabel(char*, label*);
byte validLblChar(char);

//Run Functions
void run();

//Stack
void push(uint32);
uint32 pop();
byte verifyStack(); 

//Maths
byte getDigit();

//User input/output
void printChar(short int);

short int getNumber(); 
char* getUserInput();
byte validate(char*);
short int parseNumber(char*);


//          $$$$$$$$$$$$$$$$$$
//          $ Main Functions $
//          $$$$$$$$$$$$$$$$$$



int main(int argc, char* argv[]) {

 printf("\n");
 printf(" ___                       _ _\n");
 printf("|_ _|_ __  ___  __ _ _ __ (_) |_ _   _\n");
 printf(" | || '_ \\/ __|/ _` | '_ \\| | __| | | |\n");
 printf(" | || | | \\__ \\ (_| | | | | | |_| |_| |\n");
 printf("|___|_| |_|___/\\__,_|_| |_|_|\\__|\\__, |\n");
 printf("       Programming Language      |___/\n");
 printf("\n     Created by Bryan McClain\n");
 printf("       (C) Comprosoft 2017\n\n");

  //Read all files
  for (int i = 1; i < argc; i++) {

	//Make sure file is lefit
	if( access(argv[i], F_OK ) != -1 ) {
   	  
  	  printf("\nProgram: %s\n\n", argv[i]);

	  //Open the file
	  FILE *fp = fopen(argv[i], "rb");

	  //Convert into program (Make sure it works)
	  if (parseFile(fp) == true) {
	 	pauseProgram();
		run();
	  } else {
		parseFail();
	  }

	  //Close for next file
	  fclose(fp);

	} else {
    	  printf("Error! Invalid file %s\n\n", argv[i]);
	}

  }

  return 0;


}


//##################################
//# Clear computer to default state
//##################################
void reset() {

  sys.acc = 0;
  sys.bak = 0;
  sys.pc = 0;
  sys.sp = 99;
  sys.stackProb = 0;

  sys.overflow = false;
  sys.compare = false;

  sys.mc = 0;
  sys.dig = 0;

  //Clear memory and stack
  for (int i = 0; i < 1000; i++) {
	sys.memory[i] = 0;
  }

  for (int i = 0; i < 100; i++) {
	sys.stack[i] = 0;
  }

  //Seed random numbers
  srand(time(NULL));
}





//#################################
//# Draw program debug information
//#################################
void drawDebug(label* allLabels, byte* program) {

  //Display labels found
  printf("Labels Found: %d\n",lblCount);
  for (int i = 0; i < lblCount; i++) {
	printf("  \"%s\" ",allLabels[i].name);
  	if (allLabels[i].used == false) {printf("\t(Unused)");}
	printf("\n");
  }

  //Grammer OCD
  if (curByte == 1) {
      printf("\nProgram: 1 Byte\n  ");
  } else {
      printf("\nProgram: %d Bytes\n  ",curByte);
  }

  for(int i = 0; i < curByte; i++) {
	printf("%d",sys.program[i]);
  	if (i < curByte-1) {printf(",");}
  }
  printf("\n\n\n");


}


//#################################
//# Press any key to continue...
//#################################
void pauseProgram() {

  printf("Press any key to continue...");

  getchar();	

  printf("\n");
}




//#################################
//# Dump everything to screen
//#   for debugging, dough
//#################################
void debugPause() {

  printf("\nDebug Info:\n");
  printf("-----------\n\n");
  printf("Acc: %d    \t|",sys.acc);
  printf("   Bak: %d\n",sys.bak);
  printf("PC: %d    \t|", sys.pc);
  printf("   SP: %d\n\n",sys.sp);
  printf("Memory: %d\t|",sys.mc);
  printf("   Digit: %d\n",getDigit());
  printf("Overflow: %d\t|",sys.overflow);
  printf("   Compare: %d\n",sys.compare);
  printf("\nPress any key to continue...");

  getchar();	

  printf("\n");
}


//          $$$$$$$$$$$$$$$$$$$$
//          $ Binary Functions $
//          $$$$$$$$$$$$$$$$$$$$




//#################################
//# Store a word into 4 bytes of
//#  memory, where memory is byte*
//#################################
void storeWord(byte* location, uint32 val) {
	location[0] = (byte) (val >> 24) & 0xff;
	location[1] = (byte) (val >> 16) & 0xff;
	location[2] = (byte) (val >> 8) & 0xff;
	location[3] = (byte) (val & 0xff);
}






//#################################
//# Get 4 bytes from memory, where
//#  memory is byte*
//#################################
uint32 getWord(byte* location) {
	uint32 val = 0;
	val |= (uint32) (location[0] << 24);
	val |= (uint32) (location[1] << 16);
	val |= (uint32) (location[2] << 8);
	val |= (uint32) (location[3]);
	return val;
}








//          $$$$$$$$$$$$$$$$$$
//          $ File Functions $
//          $$$$$$$$$$$$$$$$$$



//################################
//# Read the contents of the file
//#   and parse into a program
//################################
byte parseFile(FILE *fp) {
  
  //Read memory 100 bytes at a time
  sys.program = calloc(100, sizeof(byte));

  curByte = 0;		//Where am I???
  maxByte = 100;	//Where is the end of the program???

  //Stuff for labels
  label* allLabels = malloc(0);	  //Definitions for all labels found
  label* toResolve = malloc(0);   //All labels that need to be resolved in program
  lblCount = 0;			  //Number of labels in total
  resolveCount = 0;		  //Number of labels to be resolved

  uint32 lastIf = -1;	//What was the address of the last bracket {}
  uint32 ifCount = 0;	//The total number of if statements


  //Some other variables
  byte command;
  char* newName;
  byte error = false;	//Did an error occur?? (Show all error messages)

  //-----------Read all characters in the file-----------
  int c = fgetc(fp);
  while (c != EOF) {

    switch (c) {
	case ':':	//Label Definition

	  //Verify that this label doesn't already exist
	  newName = readLabel(fp,':'); 
	  if (duplicateLabel(newName, allLabels) == true) {
		printf("Error! Duplicate label \"%s\"\n", newName);
		error = true;
		break;
	  }

	  //Add to the list of global labels
	  allLabels = realloc(allLabels, (++lblCount) * sizeof(label));	//Expand buffer
	  allLabels[lblCount - 1].name = newName;
	  allLabels[lblCount - 1].address = curByte;
	  allLabels[lblCount - 1].used = false;
	  break;

	case '(':	//Jump definition

	  //Add to the list of jumps to resolve
	  toResolve = realloc(toResolve, sizeof(label) * (++resolveCount));	//Expand buffer
	  toResolve[resolveCount - 1].name = readLabel(fp,')');
	  toResolve[resolveCount - 1].address = curByte + 1;

	  sys.program[curByte] = 1;		//Type the command
	  expandProgram(&sys.program,5);		//5 total bytes needed to store a jump
	  break;

	case '[':	//Subroutine Definition

	  toResolve = realloc(toResolve, sizeof(label) * (++resolveCount) );	//Expand buffer
	  toResolve[resolveCount - 1].name = readLabel(fp,']');
	  toResolve[resolveCount - 1].address = curByte + 1;

	  sys.program[curByte] = 2;		//Type the command
	  expandProgram(&sys.program,5);		//5 total bytes needed to store a subroutine
	  break;

	case '{':	//If statement start

	  sys.program[curByte] = 24;		//Type the command
	  expandProgram(&sys.program,5);	//5 bytes needed in total

	  //Now go back and store lastIf in the 4 bytes
	  storeWord(&sys.program[curByte-4], lastIf);

	  //And load the current address into lastIf
	  lastIf = curByte-4;
	  ifCount++;
	  break;

	case '}':	//If statement end
	  if (ifCount > 0) {
		//Pop the value stored in the last if statement
	  	uint32 temp = getWord(&sys.program[lastIf]);
		
		//And store the current address into the last if statement
		storeWord(&sys.program[lastIf], curByte);

		lastIf = temp;
		ifCount--;
	  } else {
		printf("Erorr! Missing starting bracket {.\n");
		error = true;
 	  }
	  break;

	default:	//All other commands

	  command = charToCommand(c);
	
	  //Make sure it is a valid command
	  if (command != 255) {
		sys.program[curByte] = command;
		expandProgram(&sys.program,1);	//Command takes up 1 byte		
	  }

	  break;
    }
    c = fgetc(fp);
  }

  //Test for missing if end
  if (ifCount > 0) {
	printf("Error! Missing ending bracket }.\n");
	error = true;
  }
  

  //Stop at this point, before trying to resolve labels
  if (error == true) {return false;}

  //Now resolve labels
  for (int i = 0; i < resolveCount; i++) {
	uint32 addr = findLabel(toResolve[i].name, allLabels);

	if (addr == -1) {
		printf("Error! Undefined label \"%s\"\n",toResolve[i].name);
		error = true;	
	} else {
	 	storeWord(&sys.program[toResolve[i].address],addr);
	}
  }

  //Check again for errors
  if (error == true) {return false;}

  //Always end the program with a stop
  sys.program[curByte] = 255;
  expandProgram(&sys.program,1);

  //Now display compiling information
  drawDebug(allLabels, sys.program);

  //Free the leftovers
  free(allLabels);
  free(toResolve);

  return true;

}


//###############################
//# Program is unable to compile
//#    This is VERY BAD!!!
//###############################
void parseFail() {
  printf("\nUnable to compile program.\n  *Don't go insane fixing your code :)\n\n");
}




//###############################
//# Figures out when to allocate
//#  more memory for the program
//###############################
void expandProgram(byte** program, byte toExpand) {
  curByte+=toExpand;
  if (curByte >= maxByte) {
	maxByte+=100;
  	*program = realloc(*program, sizeof(byte) * maxByte);
  }

  //!!!!!!!!!!!!!NOTE!!!!!!!!!!!!
  //Program is a pointer to a pointer, so modify
  //   the pointer to that pointer, if that makes sense...
  // --Keep in mind for future code--
}






//##########################
//# Convert a char into a
//#   numeric command
//##########################
byte charToCommand(char input) {

  //Find the command in the list
  for (int i = 0; i < 28; i++) {
    if (commands[i] == input) {
	return i;
    }
  }
  return -1;
}



//          $$$$$$$$$$$$$$$$$$$
//          $ Label Functions $
//          $$$$$$$$$$$$$$$$$$$




//################################
//# Read the label and copy into
//#  a character array
//################################
char* readLabel(FILE* fp, char terminator) {

  //Create a buffer
  char* buffer = calloc(0, sizeof(char));
  uint32 buf_pos = 0;		//Where to put the character in the buffer
  uint32 buf_length = 10;	//What is the size of the buffer

  //Read until end of the file
  char c = fgetc(fp);
  while ((c != EOF) && (c != terminator)) {
	if (validLblChar(c) == true) {
		buffer[buf_pos] = c;
		buf_pos++;

		//Test when to increase buffer size
		if (buf_pos >= buf_length) {
			buf_length+=10;
			buffer = realloc(buffer, sizeof(char) * buf_length);
		}
	}
	c = fgetc(fp);
  }
  //Add the terminating character
  buffer[buf_pos++] = '\0';
  buffer = realloc(buffer, sizeof(char) * buf_pos);  

  return buffer;
}





//################################
//# Make sure no duplicate labels
//#   exist in the program
//################################
byte duplicateLabel(char* input, label* allLabels) {
  
  for (int i = 0; i < lblCount; i++) {
	if (strcmp(input, allLabels[i].name) == 0) {
		return true;
	}
  }
  return false;
}


//################################
//# Return true if input is a 
//#  letter or a number
//################################
byte validLblChar(char input) {

  if ((input >= 48 && input <= 57) ||
      (input >= 65 && input <= 90) ||
      (input >= 97 && input <= 122)) {
	return true;
  } else {
	return false;
  }
}





//################################
//# Return the address of a label
//################################
uint32 findLabel(char* input, label* allLabels) {
  
  for (uint32 i = 0; i < lblCount; i++) {
	if (strcmp(input, allLabels[i].name) == 0) {
		allLabels[i].used = true;	//Indicate that the label was used:
		return allLabels[i].address;
	}
  }
  return -1;
}




//          $$$$$$$$$$$$$$$$$
//          $ Run Functions $
//          $$$$$$$$$$$$$$$$$



//################################
//# Run the code like a boss!!!
//################################
void run() {

  //Clear the screen and reset program
  system("clear");
  reset();

  byte c;	//Stores the command
  uint16 temp;	//Temp variable
  int maths;	//Temp variable to verify add/subtract

  top:
    c = sys.program[sys.pc];
    //printf("%d\n",c);
    switch (c) {
	case 0:		//Quit
	  printf("\n\nProgram stopped at %d.\n\n", sys.pc);
	  pauseProgram();
	  return;

	case 1:		//Jump
	  sys.pc = getWord(&sys.program[sys.pc+1]) - 1;
	  break;

	case 2:		//Jump subroutine
	  push(sys.pc);
	  if (verifyStack() == false) {return;}
	  sys.pc = getWord(&sys.program[sys.pc+1]) - 1;
	  break;

	case 3:		//Return from subroutine
	  sys.pc = pop() + 4;
	  if (verifyStack() == false) {return;}
	  break;

	case 4: 	//Memory cursor right >
	  if (sys.mc < 999) {sys.mc++;}	//Cursor lock
	  break;
	
	case 5: 	//Memory cursor left <
	  if (sys.mc > 0) {sys.mc--;}
	  break;

	case 6:		//Increase digit counter
	  if (sys.dig < 2) {sys.dig++;}
	  break;
	
	case 7:		//Decrease digit counter
	  if (sys.dig > 0) {sys.dig--;}	
	  break;

	case 8: 	//Reset digit/memory counter
	  sys.dig = 0;
	  sys.mc = 0;
	  break;

	case 9:		//Upload
	  sys.acc = sys.memory[sys.mc];
	  break;

	case 10:	//Swp memory and acc
	  temp = sys.acc;
	  sys.acc = sys.memory[sys.mc];
	  sys.memory[sys.mc] = temp;
	  break;

	case 11:	//Save acc to backup
	  sys.bak = sys.acc;
	  break;
	
	case 12:	//Swap acc and backup
	  temp = sys.acc;
	  sys.acc = sys.bak;
	  sys.bak = temp;
	  break;

	case 13:	//Add
	  maths = sys.acc + getDigit();
	  if (maths > 999) {
		sys.acc = 999;
		sys.overflow = true;
	  } else {
		sys.acc = maths;
		sys.overflow = false;	
	  }
	  break;

    	case 14:	//Subtract
	  maths = sys.acc - getDigit();
	  if (maths < -999) {
		sys.acc = -999;
		sys.overflow = true;
	  } else {
		sys.acc = maths;
		sys.overflow = false;	
	  }
	  break;
	
	case 15:	//Add acc and backup
	  maths = sys.acc + sys.bak;
	  if (maths > 999) {
		sys.acc = 999;
		sys.overflow = true;
	  } else if (maths < -999) {
		sys.acc = -999;
		sys.overflow = false;
	  } else {
		sys.acc = maths;
	  	sys.overflow = false;
	  }
	  break;

	case 16:	//Negate accumulator
	  sys.acc *= -1;
	  break;

	case 17:	//Reset acc
	  sys.acc = 0;
	  break;

	case 18:	//Random number
	  sys.acc = (rand() % 1999) - 999;
	  break;

	case 19:	//EQU
	  sys.compare = (sys.acc == 0);
	  break;

	case 20:	//NEQ
	  sys.compare = (sys.acc != 0);
	  break;

	case 21:	//GTR
	  sys.compare = (sys.acc > 0);
	  break;

	case 22:	//LSS
	  sys.compare = (sys.acc < 0);
	  break;
	
	case 23:	//Overflow
	  sys.compare = sys.overflow;
	  break;

	case 24:	//If branch
	  if (sys.compare == 0) {
 		sys.pc = getWord(&sys.program[sys.pc+1]) - 1;	//Skip it
	  } else {
	    	sys.pc+=4;		//DO IT
	  }
	  break;

	case 25:	//User input
 	  sys.acc = getNumber();
	  break;

	case 26:	//Output text char
	  printChar(sys.acc);
	  break;
	
	case 27:	//Pause w/ debug
	  debugPause();
	  break;

	case 255:	//End of the program
	  printf("\n\nReached the end of the program.\n\n");
	  pauseProgram();
	  return;

    }
  sys.pc++;	//Add 1 to program counter
  goto top;
}



//################################
//# Push subroutine onto stack
//################################
void push(uint32 input) {
  sys.stack[sys.sp] = input;
  sys.sp--;
  if (sys.sp > 99) {
	sys.sp = 99;
	sys.stackProb = 1;}     //Only 100 spots on the stack (overflow)
}

//####################################
//# Pop the last value from the stack
//####################################
uint32 pop() {
  sys.sp++;
  if (sys.sp > 99) {
	sys.sp = 0;
	sys.stackProb = 2;}	//Only 100 spots on the stack (underflow)
  return sys.stack[sys.sp];
}


//####################################
//# Make sure stack is working fine
//#  True = Great :)   false = BAD!!!
//####################################
byte verifyStack() {
  switch (sys.stackProb) {
	case 0:
	  return true;
	 
	case 1:
	  printf("\n\nError! Stack Overflow!\n\n");
	  return false;

	case 2:
	  printf("\n\nError! Stack Underflow!\n\n");
	  return false;

  	case 3:
 	  return false;
  } 
}


//################################
//# Return 1, 10, or 100
//################################
byte getDigit() {
  switch (sys.dig) {
	case 0:
	  return 1;
	case 1:
	  return 10;
	case 2:
	  return 100;
	default:
	  return 1;
  }
}


//################################
//# Print the char associated with
//#  a number
//################################
void printChar(short int input) {

  if (input >= 0 && input < 94) {
	printf("%c", (char) input+32);
  } else if (input >= 94) {
	printf("☺");	//Smiley Face
  } else if (input == -1) {
	printf("\n");	//Enter
  } else if (input == -999) {
	system("clear");	//Clear terminal
  } else {
	printf("☹");	//Sad face
  }



}




//###################################
//# Return a number inputed by user
//#  Has built in messages, etc.
//###################################
short int getNumber() {

  //Step 1: ask user to input number
  printf("\nPlease enter a number: (-999 to 999)\n");

inputLoop:
  printf("-> ");
  char* input = getUserInput();
 
  if (validate(input) == false) {
	printf("\nError! Invalid Number!\n\n");
	goto inputLoop;
  }

  //Number is valid. Now parse
  printf("\n\n");

  return parseNumber(input);
}




//################################
//# Get user input as a string and
//#  allocate exact right size
//################################
char* getUserInput() {
  size_t size = 4;	//Default buffer length
  size_t len = 0;	//The actual buffer length	

  //Buffer that will hold the string
  char* str = malloc(sizeof(char) * size);
  char c;

  //Loop until STDIN is exhausted
  while (EOF != (c = getchar()) && c != '\r' && c != '\n') {

	//Copy the character into the temp buffer
	str[len++] = c;
	
	//Decide when to increase buffer size
	if (len >= size) {
		str = realloc(str, sizeof(char) * (size *= 2));
	}
  }

  //Terminate the string
  str[len++]='\0';

  //And resize to final size
  return realloc(str, sizeof(char) * len);
}



//################################
//# Verify that string is number
//################################
byte validate(char *a) {

  if (a[0] == '-') {		//NEGATIVE

	//Length should not be more than 4 characters
	// or less than 2 characters when negative
  	if (strlen(a) > 4 || strlen(a) < 2) {
		return false;
  	}

	//Now make sure all other characters are numeric
  	for (int x = 1; x < strlen ( a ); x++ ) {
    		if ( !isdigit ( a[x] ) ) return false;
	}
	
	//This number is indeed legit
  	return true;
	

  } else {			//POSITIVE

	//Length should not be more than 3 characters
	// or less than 1 characters when positive
  	if (strlen(a) > 3 || strlen(a) < 1) {
		return false;
  	}

	//Now make sure all other characters are numeric
  	for (int x = 0; x < strlen ( a ); x++ ) {
    		if ( !isdigit ( a[x] ) ) return false;
	}
	
	//This number is indeed legit
  	return true;
  }
}




//################################
//# Parse a string into a number
//#   (Please verify first!!!)
//################################
short int parseNumber(char* input) {

  short int retVal;

  //First, figure out if number is negative
  byte isNegative = false;
  if (input[0] == '-') {
	isNegative = true;
	input++;   //move the pointer
  }

  //Next, figure out the starting digit   (100, 10, 1)
  byte digit = 0;
  digit = strlen(input);

  //Do a bit of ascii conversion too
  switch(digit) {
	case 1:		//1's
	  retVal = input[0] - 48;
 	  break;

	case 2:		//10's
	  retVal = (input[0] - 48) * 10;
	  retVal += (input[1] - 48);
  	  break;
	case 3:		//100's
	  retVal = (input[0] - 48) * 100;
	  retVal += (input[1] - 48) * 10;
	  retVal += (input[2] - 48);
  	  break;
  }

  //Finally, add the negative sign
  if (isNegative == true) {
	retVal *= -1;
  }

  return retVal;
}
