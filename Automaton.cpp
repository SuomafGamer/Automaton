/* 
Created By: Bryce Knoblauch (August 18th, 2011)
Website: http://www.godlygamer.com/
Last Updated: July 3rd, 2012
*/

// Define For WIN32
#define _WIN32_WINNT 0x0501
// Library Includes
#include <stdio.h>
#include <windows.h>
#include <winuser.h>
#include <windowsx.h>

// Definitions
#define TINYBUF 32
#define SMALLBUF 128
#define BUFSIZE 512

#define FAIL 0
#define KEY 1
#define TXT 2
#define WAIT 3
#define VAR 4
#define VAR_SET 5
#define VAR_ADD 6
#define VAR_SUB 7
#define END 8
#define CLICKS 9
#define CPCLICKS 10

// Character Arrays
char myInput[SMALLBUF] = "";
char myTimer[SMALLBUF] = "";
char fileName[SMALLBUF] = "";
char automate[BUFSIZE] = "";
char pre[TINYBUF] = "";
char post[BUFSIZE] = "";

// Various Flags
int captureKey = 0;
int capturedKey = 0;
int myInt = 0;
int lastTime = 0;

int prgmActive = 0;
int loop = 0;
int looping = 0;
int executeKey = 0;
int executeAuto = 0;

// Cursor Positions
POINT cursorPos;
int mouseX1 = 0;
int mouseY1 = 0;
int mouseX2 = 0;
int mouseY2 = 0;

// Timing Variables
DWORD tstart = 0, tend = 0, tdif = 0;

DWORD TimeSinceLastCall() {
    tend = GetTickCount();
    tdif = tend - tstart;
    tstart = GetTickCount();
    return tdif;
}

void LeftClick() {  
  INPUT    Input={0};
  // left down 
  Input.type = INPUT_MOUSE;
  Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  ::SendInput(1,&Input,sizeof(INPUT));

  // left up
  ::ZeroMemory(&Input,sizeof(INPUT));
  Input.type = INPUT_MOUSE;
  Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
  ::SendInput(1,&Input,sizeof(INPUT));
}

void RightClick() {  
  INPUT    Input={0};
  // left down 
  Input.type = INPUT_MOUSE;
  Input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
  ::SendInput(1,&Input,sizeof(INPUT));

  // left up
  ::ZeroMemory(&Input,sizeof(INPUT));
  Input.type = INPUT_MOUSE;
  Input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
  ::SendInput(1,&Input,sizeof(INPUT));
}

// Synthesize Input For The Given Key Value
void keyInput(const int keyValue) {
    // Create a Key Object
    INPUT *key;
	key = new INPUT;
	key->type = INPUT_KEYBOARD;
	key->ki.wVk = VkKeyScan(keyValue);
	key->ki.dwFlags = 0;
	key->ki.time = 0;
	key->ki.wScan = 0;
	key->ki.dwExtraInfo = 0;
	// Key Down
	key->ki.dwFlags = 0;
	SendInput(1,key,sizeof(INPUT));
	// Key Up
	key->ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1,key,sizeof(INPUT));
}

// Type an Array of Characters
void type_msg(const char *msg) {
    // Number of Characters to Loop Through
    int i;
	
    // Create a Shift Key Object
    INPUT *key_shift;
	key_shift = new INPUT;
	key_shift->type = INPUT_KEYBOARD;
	key_shift->ki.wVk = 0xA0;
	key_shift->ki.dwFlags = 0;
	key_shift->ki.time = 0;
	key_shift->ki.wScan = 0;
	key_shift->ki.dwExtraInfo = 0;
	
    // Create a Key Object
    INPUT *key;
	key = new INPUT;
	key->type = INPUT_KEYBOARD;
	key->ki.wVk = 0x0D;
	key->ki.dwFlags = 0;
	key->ki.time = 0;
	key->ki.wScan = 0;
	key->ki.dwExtraInfo = 0;
    
    // Loop Through Each Character in The Array
	for (i = 0; i < strlen(msg); i++) {
        // Sleep for 3ms
        Sleep(3);

        // Set Key to Synthesize
        key->ki.wVk = VkKeyScan(msg[i]); // i.e. 0x0D;

        // Synthesize Shift Down
        if (key->ki.wVk > 256) {
    	    key_shift->ki.dwFlags = 0;
            SendInput(1,key_shift,sizeof(INPUT));
        }
        
        // Synthesize Key Down
	    key->ki.dwFlags = 0;
        SendInput(1,key,sizeof(INPUT));
        
        // Synthesize Key Up
       	key->ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1,key,sizeof(INPUT));
        
        // Synthesize Shift Up
        if (key->ki.wVk > 256) {
    	    key_shift->ki.dwFlags = KEYEVENTF_KEYUP;
            SendInput(1,key_shift,sizeof(INPUT));
        }
	}
}

// Scan For User Input
int get_input(const char prompt[], char word[], size_t n) {
    char testW[BUFSIZE];
    // Loop Until We Break (No More Input)
    while (1) {
        // Print the Prompt Text
        printf("\n%s", prompt);
        // Get User Input
        if (fgets(testW, BUFSIZE, stdin) != NULL) {
            // Remove Newline Character From Input
            testW[strlen(testW)-1] = '\0';
            // Verifies Input Size, Then Copies it to 'word'
            if (strlen(testW) < n) {
                strcpy(word, testW);
                return 1;
            }
        } else {
            break;
        }
    }
    return 0;
}

// Matches User Input to its Corresponding Command
int checkInput() {
	get_input("Enter Command: ", myInput, SMALLBUF);
	if (strcmp(myInput, "key") == 0) {
		return KEY;
	} if (strcmp(myInput, "text") == 0) {
		return TXT;
	} if (strcmp(myInput, "wait") == 0) {
		return WAIT;
	} if (strcmp(myInput, "var") == 0) {
		return VAR;
	} if (strcmp(myInput, "var set") == 0) {
		return VAR_SET;
	} if (strcmp(myInput, "var add") == 0) {
		return VAR_ADD;
	} if (strcmp(myInput, "var sub") == 0) {
		return VAR_SUB;
	} if (strcmp(myInput, "end") == 0) {
		return END;
	} if (strcmp(myInput, "clicks") == 0) {
		return CLICKS;
	} if (strcmp(myInput, "copy clicks") == 0) {
		return CPCLICKS;
	}
	return FAIL;
}

// Print a Number as a Character
// Note: This function is slightly flawed, only works into 1000's
char* numAsChar() {
    char s[TINYBUF] = "";
    int charNo = 0;
    int curNo = 0;
    int tNum = myInt;
    
    while (tNum >= 1000) {
        tNum -= 1000;
        curNo++;
    }
    if (curNo > 0) {
        // Note: 48 is 0 in ASCII
        s[charNo] = curNo+48;
        charNo++;
        curNo = 0;
    }
    
    while (tNum >= 100) {
        tNum -= 100;
        curNo++;
    }
    if (curNo > 0) {
        s[charNo] = curNo+48;
        charNo++;
        curNo = 0;
    }
    
    while (tNum >= 10) {
        tNum -= 10;
        curNo++;
    }
    if (curNo > 0) {
        s[charNo] = curNo+48;
        charNo++;
        curNo = 0;
    }
    
    while (tNum >= 1) {
        tNum -= 1;
        curNo++;
    }
    s[charNo] = curNo+48;
    charNo++;
    curNo = 0;
    
    s[charNo] = '\0';
    return s;
}

// Thread For Capturing Command Keys
DWORD WINAPI myThread(LPVOID args) {
    int i = 0;
	int key = 0;
	int keyed = 0;
	
	while (1) {
        Sleep(10);
		// Program Active
		if (prgmActive) {
			// Check Execute Key
			if (GetAsyncKeyState(executeKey) == 0) {
				key = 0;
				if (keyed == 1) {
					keyed = 0;
					// Released KEY!
					if (loop) {
						if (looping) {
							looping = 0;
							printf("TOGGLE OFF!");
						} else {
							looping = 1;
							printf("TOGGLE ON!");
						}
					} else {
						printf("EXECUTE!");
						executeAuto = 1;
					}
				}
			} else {
				key = 1;
				if (keyed == 0) {
					keyed = 1;
					// Pressed KEY!
				}
			}
		}
		// Capture Key
		if (captureKey) {
			for (i = 0; i < 256; i++) {
				if (GetAsyncKeyState(i) == 0xFFFF8001) {
					// STORE PRESSED KEY
					capturedKey = i;
					captureKey = 0;
					// Store Mouse Position
					GetCursorPos(&cursorPos);
					mouseX1 = (int)cursorPos.x;
					mouseY1 = (int)cursorPos.y;
				}
			}
		}
	}
}

// Carries Out The Instructions to Automate
void automateIt() {
	int i;
	int location = 0;
	int seperated = 0;
	int tempNo1;
	int tempNo2;
	char tempChar[TINYBUF];
	// Carry out the Command
	for (i = 0; i <= strlen(automate); i++) {
		if (!seperated) {
			pre[location++] = automate[i];
		} else {
			post[location++] = automate[i];
		}
		if (automate[i] == ':') {
			seperated = 1;
			pre[--location] = '\0';
			location = 0;
		}
	}
	// Variable "pre" contains the command, and "post" contains the data. (i.e. "TEXT" "Hello World!")
	if (strcmp(pre, "KEY") == 0) {
		if (sscanf(post, "%d", &tempNo1) == 1) {
			keyInput(tempNo1);
		}
	} else if (strcmp(pre, "LCLICK") == 0) {
		if (sscanf(post, "%d %d", &tempNo1, &tempNo2) == 2) {
			SetCursorPos(tempNo1, tempNo2);
			LeftClick();
		}
	} else if (strcmp(pre, "RCLICK") == 0) {
		if (sscanf(post, "%d %d", &tempNo1, &tempNo2) == 2) {
			SetCursorPos(tempNo1, tempNo2);
			RightClick();
		}
	} else if (strcmp(pre, "TEXT") == 0) {
		type_msg(post);
	} else if (strcmp(pre, "VAR") == 0) {
		strcpy(tempChar, numAsChar());
		type_msg(tempChar);
	} else if (strcmp(pre, "SET") == 0) {
		if (sscanf(post, "%d", &tempNo1) == 1) {
			myInt = tempNo1;
		}
	} else if (strcmp(pre, "ADD") == 0) {
		if (sscanf(post, "%d", &tempNo1) == 1) {
			myInt += tempNo1;
		}
	} else if (strcmp(pre, "SUB") == 0) {
		if (sscanf(post, "%d", &tempNo1) == 1) {
			myInt -= tempNo1;
		}
	} else if (strcmp(pre, "TIME") == 0) {
		if (sscanf(post, "%d", &tempNo1) == 1) {
			Sleep(tempNo1);
		}
	}
}

// Handles the Looping From End of File To Start
void handleLoop(FILE *fp) {
	while (1) {
		Sleep(10);
		// If Toggled On
		if (looping) {
			// Do Next Command
			if (fgets(automate, sizeof automate, fp) != NULL && automate[0] != '\0') {
				automate[strlen(automate)-2] = '\0';
				// Process Command
				automateIt();
			} else {
				fseek(fp, 0, SEEK_SET);
			}
		} else {
			fseek(fp, 0, SEEK_SET);
		}
	}
}

// Handles the Execution of Individual Lines
void handleExecute(FILE *fp) {
	while (1) {
		Sleep(100);
		if (executeAuto) {
			executeAuto = 0;
			// Go Through Commands
			while (fgets(automate, sizeof automate, fp) != NULL) {
				Sleep(10);
				automate[strlen(automate)-2] = '\0';
				// Process Command
				automateIt();
			}
			fseek(fp, 0, SEEK_SET);
		}
	}
}

// Function to Capture Singular Keystrokes
void captureKeyStroke() {
	Sleep(500);
	captureKey = 1;
	printf("Listening for Keystroke...");
	while (captureKey) {
		Sleep(50);
	}
	printf(" Captured Key: %d", capturedKey);
	Sleep(500);
}

// Writes The Captured Key to The Automaton File
void writeCapturedKey(FILE *fp) {
	captureKeyStroke();
	if (capturedKey == 1) {
		fprintf(fp, "LCLICK:%d %d\r\n", mouseX1, mouseY1);
	} else if (capturedKey == 2) {
		fprintf(fp, "RCLICK:%d %d\r\n", mouseX1, mouseY1);
	} else {
		fprintf(fp, "KEY:%d\r\n", capturedKey);
	}
}
// Writes the Captured Text to The Automaton File
void captureText(FILE *fp) {
	get_input("Enter Text: ", myInput, SMALLBUF);
	fprintf(fp, "TEXT:%s\r\n", myInput);
}
// Writes the Captured Time to The Automaton File
void captureTime(FILE *fp) {
	get_input("Enter Time to Wait (miliseconds): ", myInput, SMALLBUF);
	fprintf(fp, "TIME:%s\r\n", myInput);
}
// Writes the Variable Print Command to The Automaton File
void printVAR(FILE *fp) {
	printf("Print Variable Number.");
	fprintf(fp, "VAR:\r\n");
}
// Writes the Variable Set Command to The Automaton File
void varSET(FILE *fp) {
	get_input("Set Var To (Integer): ", myInput, SMALLBUF);
	fprintf(fp, "SET:%s\r\n", myInput);
}
// Writes the Variable Add Command to The Automaton File
void varADD(FILE *fp) {
	get_input("Add To Var (Integer): ", myInput, SMALLBUF);
	fprintf(fp, "ADD:%s\r\n", myInput);
}
// Writes the Variable Subtract Command to The Automaton File
void varSUB(FILE *fp) {
	get_input("Subtract From Var (Integer): ", myInput, SMALLBUF);
	fprintf(fp, "SUB:%s\r\n", myInput);
}
// Function to Capture Mouse Clicks
// Writes the Intended Captured Clicks to The Automaton File
void captureClicks(FILE *fp) {
    bool capturing = true;
    int myCounter = 0;
    // Prompt For Delay
	get_input("Enter Time to Wait Between Clicks (miliseconds): ", myTimer, SMALLBUF);
    // Capture Clicks Until [BACKSPACE] Pressed
    while (capturing) {
        // Set To Capture
        captureKey = 1;
        // Wait For Input
        while (captureKey) {
            Sleep(50);
        }
        // Caught Left Click
        if (capturedKey == 189) {
            fprintf(fp, "LCLICK:%d %d\r\n", mouseX1, mouseY1);
            if (strcmp(myTimer, "0") != 0) { fprintf(fp, "TIME:%s\r\n", myTimer); }
            printf("Captured a Left-Click ([BACKSPACE] TO STOP CAPTURING) %d\n", myCounter++);
        // Caught Right Click
        } else if (capturedKey == 187) {
            fprintf(fp, "RCLICK:%d %d\r\n", mouseX1, mouseY1);
            if (strcmp(myTimer, "0") != 0) { fprintf(fp, "TIME:%s\r\n", myTimer); }
            printf("Captured a Right-Click ([BACKSPACE] TO STOP CAPTURING)%d\n", myCounter++);
        // Caught [BACKSPACE]
        } else if (capturedKey == 8) {
            capturing = false;
            printf("Capturing Clicks Has Been Stopped.\n");
        }
    }
}
// Function to Copy Mouse Clicks
// Writes the Intended Captured Clicks to The Automaton File
void copyClicks(FILE *fp) {
    bool capturing = true;
    bool copying = false;
    DWORD delayTimer = 0;
    // Capture Clicks Until [BACKSPACE] Pressed
    while (capturing) {
        // Set To Capture
        captureKey = 1;
        // Wait For Input
        while (captureKey) {
            Sleep(50);
        }
        // Toggle On
        if (capturedKey == 189) {
            copying = true;
            TimeSinceLastCall();
            printf("Start Capturing...\n");
        // Toggle Off
        } else if (capturedKey == 187) {
            copying = false;
            printf("Stop Capturing.\n");
        // Caught Left Click
        } else if (copying && capturedKey == 1) {
            delayTimer = TimeSinceLastCall();
            if (delayTimer > 0) { fprintf(fp, "TIME:%d\r\n", delayTimer); }
            fprintf(fp, "LCLICK:%d %d\r\n", mouseX1, mouseY1);
            printf("Captured a Left-Click (%d)\n", delayTimer);
        // Caught Right Click
        } else if (copying && capturedKey == 2) {
            delayTimer = TimeSinceLastCall();
            if (delayTimer > 0) { fprintf(fp, "TIME:%d\r\n", delayTimer); }
            fprintf(fp, "RCLICK:%d %d\r\n", mouseX1, mouseY1);
            printf("Captured a Right-Click (%d)\n", delayTimer);
        // Caught [BACKSPACE]
        } else if (capturedKey == 8) {
            capturing = false;
            printf("Capturing Clicks Has Been Stopped.\n");
        }
    }
}

// Prints the Useable Commands to The Console For The User
void createTemplate(FILE *fp) {
	int ended = 0;
	
	printf("You must now construct your template.\nplease use the following commands to do so:\n");
	printf("   \"key\" - this will store your next keystroke/click\n");
	printf("   \"text\" - this will store the text you type (max 127 chars)\n");
	printf("   \"clicks\" - capture left-clicks (-) and right-clicks (=)\n");
	printf("              Press [BACKSPACE] to stop capturing clicks\n");
	printf("   \"copy clicks\" - toggle on (-) toggle off (=)\n");
	printf("                   Press [BACKSPACE] to stop capturing clicks\n");
	printf("   \"wait\" - this will wait for the amount of miliseconds you specify\n");
	printf("   \"var\" - this will print an integer value (default 0)\n");
	printf("   \"var set\" - this allows you to set the variables value\n");
	printf("   \"var add\" - this allows you to increase the variables value\n");
	printf("   \"var sub\" - this allows you to decreate the variables value\n");
	printf("   \"end\" - SPECIFIES END OF SCRIPT\n");

	while (!ended) {
		switch (checkInput()) {
			case KEY:
				writeCapturedKey(fp);
				break;
			case TXT:
				captureText(fp);
				break;
			case WAIT:
				captureTime(fp);
				break;
			case VAR:
				printVAR(fp);
				break;
			case VAR_SET:
				varSET(fp);
				break;
			case VAR_ADD:
				varADD(fp);
				break;
			case VAR_SUB:
				varSUB(fp);
				break;
			case END:
				ended = 1;
				break;
			case CLICKS:
				captureClicks(fp);
				break;
			case CPCLICKS:
				copyClicks(fp);
				break;
			default:
				printf("Unrecognized Command.");
		}
		printf("\n");
	}
}

// Main Function For The Automaton Program
// Handles: Creating/Saving/Loading & Execution of Templates
int main(void) {

	int i;
	FILE *fp;

	CreateThread(NULL, 0, myThread, NULL, 0, NULL);
	
	begin:
	printf("-------------------------------------------------------\n");
	printf("If you wish to create a new template, leave this blank.");
	get_input("Load Existing Template: ", myInput, SMALLBUF);
	
	printf("\n");
	if (myInput[0] == '\0') {
		// CREATE NEW TEMPALTE
		printf("Creating new template... (if you do not wish to save it leave this blank)");
		get_input("Save Template As: ", fileName, SMALLBUF);
		if (fileName[0] == '\0') {
			strcpy(fileName, "temp.txt");
		} else {
			strcat(fileName, ".txt");
		}
		// Check if file exists (prompt overwrite if so)
		if ((fp = fopen(fileName, "rb")) != NULL) {
			get_input("Template Exists, Overwrite It? (y/n) ", myInput, SMALLBUF);
			if (myInput[0] == 'y' || myInput[0] == 'Y') {
				printf("\nOverwritting existing template...\n");
			} else {
				printf("\nYou have chosen not to overwrite existing template.\n\n");
				goto begin;
			}
			fclose(fp);
		}
		// Create an empty file for writing. If a file with the same name already exists its content is erased and the file is treated as a new empty file.
		if ((fp = fopen(fileName, "wb")) != NULL) {
			printf("\n--------------------\n");
			printf("Creating Template...\n\n");
		}
		// Begin filling out the template
		createTemplate(fp);
		fclose(fp);
		// Open a file for reading. The file must exist.
		if ((fp = fopen(fileName, "rb")) != NULL) {
			printf("\nTemplate Successfully Created!\n\n");
		} else {
			printf("\nError: Template Could Not Be Loaded!\n\n");
			goto begin;
		}
	} else {
		// LOAD EXISTING TEMPLATE
		printf("Attempting to Load Template: \"%s\"", myInput);
		strcat(myInput, ".txt");
		// Open a file for reading. The file must exist.
		if ((fp = fopen(myInput, "rb")) != NULL) {
			printf("\nTemplate Opened!\n\n");
		} else {
			printf("\nTemplate Not Found!\n\n");
			goto begin;
		}
    }
	
	// At this point, "fp" is now the file containing our instructions.
	printf("------------------------\n");
	printf("Initializing Template...\n");
	
	// Ask If Auto-Loop
	get_input("Automatically Loop When End Reached? (y/n) ", myInput, SMALLBUF);
	if (myInput[0] == 'y' || myInput[0] == 'Y') {
		loop = 1;
	} else {
		loop = 0;
	}
	// Ask For Key Initializer
	printf("The next key you press will be used as the trigger key.\n");
	if (loop) {
		printf("(Whenever this key is released it will toggle your commands on/off)\n");
	} else {
		printf("(Whenever this key is released it will execute your commands)\n");
	}
	captureKeyStroke();
	executeKey = capturedKey;
	prgmActive = 1;
	
	printf("\n\nCommands Found:\n");
	while (fgets(automate, sizeof automate, fp) != NULL) {
		printf("  %s", automate);
	}
	printf("\n");
	
	// Set Cursor To Beginning of Text File
	fseek(fp, 0, SEEK_SET);
	// Handle The Commands
	if (loop) {
		handleLoop(fp);
	} else {
		handleExecute(fp);
	}
	// PROGRAM END
	printf("\n\n");
	system("pause");
	// Close File.
	if (fp != NULL) {
		fclose(fp);
	}
    return 0;
}
