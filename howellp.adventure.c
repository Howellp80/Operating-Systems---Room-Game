/*******************************************************************************
 * howellp.advnenture.c
 * Parker Howell
 * 10-25-17
 * Description - This program references the classic "Colossal Cave Adventure" 
 * game. The game asks the user to navigate a series of rooms until the end 
 * room is found. When found, the user will be congratulated, a count of the 
 * number of rooms visited will be displayed, and the the path took to find the
 * exit (up to 50 rooms) will be displayed.
 * Additinally, the user can enter "time" at the prompt to display the current
 * time and date.
 *
 * ****************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>



// struct to define a Room
struct Room
{
	char* name;                     // what the room is called
	char* roomType;                 // start / mid / end
	int numOutConn;                 // how many rooms are connected
	struct Room* outboundConn[6];   // pointers to connected rooms
};

// globals for game data and state...
struct Room* rooms[7];     // array to hold created Room structs
struct Room* currRoom;     // holds a pointer to the current room
int steps = 0;             // counts the steps taken to reach exit
struct Room* path[50];     // saves order of first 50 rooms visited
char* input;               // stores users command input

// 7 file names
char* fileNames[7] = {"room0", "room1", "room2", "room3",
       	"room4", "room5", "room6"};

// the Mutex for thread control
pthread_mutex_t myMutex;




//***************************************************************************//
//----------------------------- Helper Function -----------------------------//
//***************************************************************************//

/*******************************************************************************
 * printRooms
 * displays the contents of the entire rooms array for testing or error 
 * checking
 *
 * ****************************************************************************/
void printRooms(void){
	int i;
	for (i = 0; i < 7; i++){
		printf("Room Name: %s\n", rooms[i]->name);
		int j;
		int conns = rooms[i]->numOutConn;
		printf("Outbound Connections: %d\n", conns);
		for (j = 0; j < conns; j++){
			printf("Connection %d: %s\n",
				       	j+1, rooms[i]->outboundConn[j]->name);
		}
		printf("RoomType: %s\n", rooms[i]->roomType);
	}
	printf("\n");
}








//***************************************************************************//
//------- The following functions are used in setting up the game data ------//
//***************************************************************************//

/*******************************************************************************
 * initRoomArray
 * Sets up the room array so we can read from files into it.
 *
 * ****************************************************************************/
void initRoomArray(void){
	//printf("In initRoomArray\n");	
	int i;
	for (i = 0; i < 7; i++){
		// allocate the memory for a room
		rooms[i] = calloc(1, sizeof(struct Room));
		// allocate the memory for the rooms name
		rooms[i]->name = calloc(15, sizeof(char));
		// allocate the memory for the rooms type
		rooms[i]->roomType = calloc(11, sizeof(char));
		// set initial outbound connections
		rooms[i]->numOutConn = 0;
	}
}



/*******************************************************************************
 * getNewestDir
 * Finds the newest created subfolder in the current directory matching the 
 * name prefix "howellp.rooms." and changes the current working directory to 
 * that so we can then read from the files within it.
 * 
 * Ref: assigned reading - "2.4 Manipulating Directories"
 *
 * ****************************************************************************/
void getNewestDir(void){
	int newDirTime = -1;    // modified timestamp of newest subdir examined
	char targetDirPre[32] = "howellp.rooms.";   // prefix we're looking for
	char newDirName[256];  // holds name of newest dir that has prefix
	
	// clear newDirName
	memset(newDirName, 0, sizeof(newDirName));

	DIR* dirToCheck;  // holds directory we're starting in
	struct dirent* fileInDir;  // holds current subdir of starting dir
	struct stat dirAttributes;  // holds info we've gained about subdir

	// open the directory we're in
	dirToCheck = opendir(".");

	// check we could open dir
	if (dirToCheck > 0){
		// check each entry
		while ((fileInDir = readdir(dirToCheck)) != NULL){
			// if entry matches prefix
			if (strstr(fileInDir->d_name, targetDirPre) != NULL){
				// get info about entry
				stat(fileInDir->d_name, &dirAttributes);
				// if current entry time is bigger set as newest
				if ((int)dirAttributes.st_mtime > newDirTime){
					newDirTime = (int)dirAttributes.st_mtime;
					memset(newDirName, 0, sizeof(newDirName));
					strcpy(newDirName, fileInDir->d_name);
				}
			}
		}
	}
	// close the directory
	closedir(dirToCheck);
	
	// change the current working directory
	chdir(newDirName);
	//printf("The new dir name is: %s\n", newDirName);
}



/*******************************************************************************
 * getRoomName
 * searches the rooms array for a Room struct with a name matching the passed 
 * in name argument. When found, returns a pointer to that Room struct.
 *
 * ****************************************************************************/
struct Room* getRoomPtr(char* name){
	//printf("searching for: %s\n", name);
	int i;
	for (i = 0; i < 7; i++){
		if(strcmp(rooms[i]->name, name) == 0)
			return rooms[i];
	}
}



/*******************************************************************************
 * buildRoomNames
 * goes through the files in howellp.rooms.### and retrieves the room name for 
 * each one and adds it to the appropriate Room struct in the rooms array.
 * We have to do this first so that later when we add the room connections
 * we can search for the Room struct with a matching name using getRoomPtr.
 *
 * ****************************************************************************/
void buildRoomNames(void){
	FILE* fp;             // file poiter for opened file
	int i;                // for loop counter
	size_t size = 32;     // bytes to read from file
	char* inLine;         // stores info read from file
	char target[20];      // the name we are looking for from file

	// for each of the 7 available room files
	for (i = 0; i < 7; i++){

		// open a file for reading
		fp = fopen(fileNames[i], "r");

		if (fp != NULL){
			// allocate memory for input
			inLine = calloc(32, sizeof(char));
		
			// get the first line with name info
			getline(&inLine, &size, fp);

			// extract the name value to target
			sscanf(inLine, "%*s %*s %s", target);
			//printf("name target is: %s\n", target);
			
			strcpy(rooms[i]->name, target);
			//printf("wrote rm name: ..%s..\n", rooms[i]->name);
		
			// close the file pointer
			fclose(fp);

			// free calloc'd memory
			free(inLine);
			
		}
	}
}



/*******************************************************************************
 * buildRestOfRooms
 * now that we are in the newest available howellp.rooms.### directory, and our 
 * room structs have room names to access them by, read from each of the files 
 * and use that info to remake the rooms array structs.
 * ****************************************************************************/
void buildRestOfRooms(void){
	FILE* fp;             // file pointer for opened file
	int i;                // for loop counter
	size_t size = 32;     // bytes to read from file
	char* inLine;         // stores input from file
	char first[20];       // stores first part of file input
	char second[20];      // stores second part of file input
	char target[20];      // stores target name/value of file input
	struct Room* rp;      // holds room pointer to a room we want to 
	                      // assign data to
		
	//printf("From File:\n");
	for (i = 0; i < 7; i++){
		
		// open a file for reading
		fp = fopen(fileNames[i], "r");
		
		// if the file opened successfully
		if (fp != NULL){
			//printf("file %s was opened\n", fileNames[i]);

			// allocate memory for inLine so we can store a line
			inLine = calloc(32, sizeof(char)); 

			// get every line from the file
			while (getline(&inLine, &size, fp) != -1){
				//printf("line string: %s", inLine);	
				
				// break each line into 3 parts
				sscanf(inLine, "%s %s %s",
					       	first, second, target);
				//printf("First:.%s.\n", first);
				//printf("Second:.%s.\n", second);
				//printf("Target:.%s.\n", target);
	
				// if we have a room connection
				if (strcmp(first, "CONNECTION") == 0) {
					
					// get/set the Room ptr matching target
					rp = getRoomPtr(target);
					rooms[i]->outboundConn[rooms[i]->numOutConn] = rp;
					
					// increment number of Room connections
					rooms[i]->numOutConn++;
				}
				// else if line is room type
				else if (strcmp(second, "TYPE:") == 0){
					// set the room type
					strcpy(rooms[i]->roomType, target);
				}
			}
			free(inLine);
		}
		fclose(fp);	
	}
	// return to the programs original working directoy so later when/if
	// someone enters "time" the file is created in the right place.
	chdir("..");
}








//***************************************************************************//
//---------- The following functions are used in running the game -----------//
//***************************************************************************//

/*******************************************************************************
 * initGame
 * sets the currRoom to the first room in rooms array, which is START_ROOM
 *
 * ****************************************************************************/
void initGame(void){
	currRoom = rooms[0];
}



/*******************************************************************************
 * addCurr
 * adds the current Room to the path array and increments steps so we can 
 * track where we've been. Only adds the first 50 rooms visited to path.
 * 
 * ****************************************************************************/
void addCurr(void){
	// avoid seg fault if player gets lost...
	if (steps <= 49){
		path[steps] = currRoom;
	}
	steps++;
}



/*******************************************************************************
 * connMatch
 * compares user input vs the current Rooms outboundconn's names. Returns true
 * if there is a name match, false otherwise.
 *
 * ****************************************************************************/
bool connMatch(char * userIn){
	//printf("In connMatch\n");
	int i;
	for (i = 0; i < currRoom->numOutConn; i++){
		if (strcmp(userIn, currRoom->outboundConn[i]->name) == 0){
			//printf("found connection match\n");
			return true;
		}
	}
	//printf("didn't find connection match\n");
	return false;
}



/*******************************************************************************
 * printPrompt
 * prints the prompt for the user to enter a command at.
 *
 * ****************************************************************************/
void printPrompt(void){
	printf("WHERE TO? >");
}



/*******************************************************************************
 * printCurr
 * prints out the current Room information - name and connections so the user
 * can decide where to go from here.
 * 
 * ****************************************************************************/
void printCurr(void){
	printf("CURRENT LOCATION: %s\n", currRoom->name);
	printf("POSSIBLE CONNECTIONS: ");
	int i;
	for (i = 0; i < currRoom->numOutConn; i++){
		printf("%s", currRoom->outboundConn[i]->name);
		
		// fomatting between rooms and after the last one
		if (i < currRoom->numOutConn - 1)
			printf(", ");
		else
			printf(".\n");
	}
}



/*******************************************************************************
 * printHuh
 * prints error message related to invalid user input
 *
 * ****************************************************************************/
void printHuh(void){
	printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
}



/*******************************************************************************
 * printEnd
 * prints out the ending messages including how many step were taken to reach
 * the end room.
 * 
 * ****************************************************************************/
void printEnd(void){
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATINS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", steps);
}



/*******************************************************************************
 * printPath
 * loops through path array and prints out the name of every room in it
 * 
 * ****************************************************************************/
void printPath(void){
	int i;
	for (i = 0; i < steps; i++){
		// prevent seg fault if user got lost...
		if (i <= 49)
			printf("%s\n", path[i]->name);
	}
}



/*******************************************************************************
 * printTime
 * opens the file "currentTime.txt" and prints out the one line of content.
 *
 * ****************************************************************************/
void printTime(void){
	//printf("In printTime\n");
	size_t size = 32;                         // bytes to read from file
	char* inLine = calloc(32, sizeof(char));  // stores data from file
	FILE* fp;                                 // file pointer to opend file

	// open the saved time file
	fp = fopen("currentTime.txt", "r");
	
	// if we could open the file
	if (fp != NULL){
		// read the only line in file and print it
		getline(&inLine, &size, fp);
		printf("\n%s\n", inLine);
		// close the file pointer
		fclose(fp);
	}
	// else error opening file
	else {
		printf("Error opening currentTime.txt for reading\n");
	}

	// free allocated memory
	free(inLine);
}



/*******************************************************************************
 * makeTimeFile
 * function executes in makeTimeThread's pthread_create call.
 * Gets the local time and writes it to the file "currentTime.txt"
 *
 * Ref: www.cplusplus.com/reference/ctime/localtime/
 *   : https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
 *
 * ****************************************************************************/
void* makeTimeFile(void* args){
	int returnVal;        // captures return of lock call
	// time thread attempts to lock Mutex
	// needs to wait for main thread to unlock on line !x!x!x!x!x!x!x!x!
	returnVal = pthread_mutex_lock(&myMutex);
	//printf("return from time thread lock call: %s\n", strerror(returnVal));
	//printf("time t locked\n");

	time_t rawtime;                              // holds time info
	struct tm* timeinfo;                         // holds converted time info
	char* formatted = calloc(64, sizeof(char));  // holds formatted time str
	FILE* fp;                                    // pointer to opened file

	// get the time
	time(&rawtime);
	// convert the time value to a struct
	timeinfo = localtime (&rawtime);
	// asctime converts the struct to a string but doesnt have 
	// desired format...
	//printf("Current local time and date: %s\n", asctime(timeinfo));
	// so...
	// take timeinfo struct and convert it to a string in the format we 
	// want. New string saved in formatted.
	strftime(formatted, 64, "%I:%M%P, %A, %B %d, %Y", timeinfo);

	// now that we have the formatted time, write it to the file
	fp = fopen("currentTime.txt", "w");

	// check if file opened
	if (fp != NULL){
		// write the time to the file
		fprintf(fp, "%s\n", formatted);
		// close the file pointer
		fclose(fp);
	}
	// else there was an error opening the file
	else {
		printf("Error opening currentTime.txt for writing\n");
	}
	
	// free allocated memory
	free(formatted);

	// print the created time file
	printTime();

	// time thread unlocks the Mutex
	pthread_mutex_unlock(&myMutex);
	//printf("time t unlocked\n");

	return NULL;
}



/*******************************************************************************
 * makeTimeThread
 * creates the time thread that will create and then print the time/date 
 * information. The thread has a very brief wait between calling for a lock and 
 * the main thread unlocking the Mutex.
 * 
 * ****************************************************************************/
void makeTimeThread(void){
	//printf("In makeTimeThread\n");		
	int resultInt;           // 0 on success
	pthread_t myThreadID;    // stores thread ID of created thread

	// start a thread for makeTimeFile()
	resultInt = pthread_create(&myThreadID,  // holds thread ID
			NULL,                    // default attributes
			makeTimeFile,            // function pointer to run
			NULL);                   // no args
	
	// main thread unlocks Mutex allowing makeTimeFile to execute
	pthread_mutex_unlock(&myMutex);
	//printf("main t unlocked\n");

	// check if create worked. Returns 0 on success
	if (resultInt != 0){
		printf("Error creating thread\n");
	}
	// else it worked
	// this is unnecessary and probably a "double block"...
	// the time thread unlocks just before returning and the main
	// thread relocks just after.
	else {
		// block main thread until makeTimeFile thread completes
		pthread_join(myThreadID, NULL);
	}

	// reLock the main thread Mutex
	pthread_mutex_lock(&myMutex);
	//printf("main t locked\n");
}



/*******************************************************************************
 * gameLoop
 * the main game loop will display the current room and give the player options
 * for which room to choose next. The game ends when the END_ROOM is reached.
 * Players may get the current time by entering "time" when prompted to 
 * choose the next room.
 * Once the end is reached a congratulatory message will be displayed, along 
 * with the amount of steps taken and the actual room path of the first 50
 * or less rooms.
 *
 * ****************************************************************************/
void gameLoop(){

	size_t size = 32;                         // bytes to read from user
	char* userIn = calloc(32, sizeof(char));  // stores user input
	bool goodInput;                           // tracks if user input valid
	
	// while we aren't at the END_ROOM
	while (strcmp(currRoom->roomType, "END_ROOM") != 0){
		// set/reset flag
		goodInput = true;
		
		// if last user input wasnt "time" print the current room.
		// On first encounter, userIn is calloc'd so should printCurr.
		if (strcmp(userIn, "time") != 0){
			printCurr();
		}

		// print user prompt
		printPrompt();

		// get the user input and strip the trailing '\n'
		getline(&userIn, &size, stdin);
		userIn[strlen(userIn) - 1] = '\0';
		//printf("The user entered: ..%s..\n", userIn);

		// if userIn matches "time"
		if (strcmp(userIn, "time") == 0){
			makeTimeThread();				
		}	
		
		// else if userIn matches a currRoom connection
		else if (goodInput = connMatch(userIn)){
			// set the current room to the user input
			printf("\n");
			currRoom = getRoomPtr(userIn);
			// add the current room to path and increment steps
			addCurr();
		}

		// check if user entered an invalid response
		if (!goodInput){
			printHuh();
		}
	}

	// we reached the END_ROOM !!! 	
	// print congrats and ending stats
	printEnd();
	printPath();

	// free user input buffer
	free(userIn);
}



/*******************************************************************************
 * freeRooms
 * loops through the rooms array and for each room struct frees the allocated
 * memory for the struct name, type, and the struct itself.
 *
 * ****************************************************************************/
void freeRooms(void){
	//printf("In freeRooms\n");
	int i;
	for (i = 0; i < 7; i++){
		free(rooms[i]->name);
		free(rooms[i]->roomType);	
		free(rooms[i]);
	}
}



/*******************************************************************************
 * main
 * controls the main flow of the program. Sets up the game data from files to 
 * an array of pointer to Room structs. Once complete, the game loop then runs,
 * exiting on completion.
 *
 * ****************************************************************************/
int main(){
	//printf("In main\n");
	
	// initialize  mutex
	pthread_mutex_init(&myMutex, NULL);
	
	// main thread locks mutex
	pthread_mutex_lock(&myMutex);
	//printf("main t locked\n");

	// setup
	initRoomArray();
	getNewestDir();
	buildRoomNames();
	buildRestOfRooms();
	//printRooms();
	
	// the game
	initGame();
	gameLoop();	
		
	// cleanup
	freeRooms();

	// main thread unlocks mutex
	pthread_mutex_unlock(&myMutex);
	//printf("main t unlocked\n");
	
	// destroy the Mutex
	pthread_mutex_destroy(&myMutex);
	
	return (0);
}







