/******************************************************************************
 * buildrooms.c
 * Parker Howell
 * 10-25-17
 * Description - This program ..... !! finish me !! .... 
 *
 * ****************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>




// struct to define a room
struct Room
{
	char* name;                     // what the room is called
	char* roomType;                 // start / mid / end
	int numOutboundConnections;     // how many rooms are connected
	struct Room* outboundCon[6];    // pointers to connected rooms
};
       
// array to hold created Room structs
struct Room* rooms[10];

// 10 available room names
char* roomNames[10] = {"Dining_Room", "Hallway", "Wine_Cellar",
	"Master_Bedroom", "Guest_Bedroom", "Study", "Balcony",
       "Kitchen", "Studio", "Foyer"};

// 7 file names      
char* fileNames[7] = {"room0","room1", "room2", "room3",
	"room4", "room5", "room6"};





//***************************************************************************//
//-------------------------  Helper Function  -------------------------------//
//***************************************************************************//

/*******************************************************************************
 * printRooms
 * a helper function to display the contents of the rooms array for testing
 * or error checking
 * ****************************************************************************/
void printRooms(int x){
	int i;
	for (i = 0; i < x; i++){
		printf("Room name: %s\n", rooms[i]->name);
		int j;
		int conns = rooms[i]->numOutboundConnections;
		printf("Outbound Connections: %d\n", conns);
		for (j = 0; j < conns; j++){
			printf("Connection %d: %s\n",
				       	j+1, rooms[i]->outboundCon[j]->name);
		}
	}
	printf("\n");
}





//***************************************************************************//
//------------ The following functions connect the room structs -------------//
//---------------------- stubs provided by professor ------------------------//
//***************************************************************************//

// returns true if all rooms have 3 or more outbound connections, 
// false otherwise
bool IsGraphFull(void){
	int i;
	for (i = 0; i < 7; i++){
		if(rooms[i]->numOutboundConnections < 3)
			return false;
	}
	return true;
}



// returns a random Room, does NOT validate if connection can be aded
struct Room* GetRandomRoom(void){
	int index = rand() % 7;
	return rooms[index];
}
	

	
// returns true if a connection can be added from Room x, false otherwise
bool CanAddConnectionFrom(struct Room* x){
	if (x->numOutboundConnections < 6)
		return true;
	else
		return false;
}
	
	

// connects Rooms x and y together, does not check if this connection is valid
void ConnectRoom(struct Room* x, struct Room* y){
	x->outboundCon[x->numOutboundConnections] = y;
	x->numOutboundConnections++;
}
		
	

// returns true if Rooms x and y are the same Room, false otherwise
bool IsSameRoom(struct Room* x, struct Room* y){
       if (x->name == y->name)
	       return true;
       else
	       return false;
}	       



// returns true if a connection from Room x to Room y already exists,
// false otherwise
bool ConnectionAlreadyExists(struct Room* x, struct Room* y){
	int i;
	for (i = 0; i < x->numOutboundConnections; i++){
		if (x->outboundCon[i]->name == y->name)
			return true;
	}
	return false;
}



// adds a random, valid outbound connection form a Room to another Room
void AddRandomConnection(void){
	struct Room* A; 
	struct Room* B;
	
	while(true){
		A = GetRandomRoom();
	
		if (CanAddConnectionFrom(A) == true)
			break;
	}
	
	do{
		B = GetRandomRoom();
	} while(CanAddConnectionFrom(B) == false 
			|| IsSameRoom(A, B) == true 
				|| ConnectionAlreadyExists(A, B) == true);
	
		ConnectRoom(A, B);
		ConnectRoom(B, A);
}



// Create all connections in graph
void connectRooms(void){
	//printf("In connectRooms\n");

	// create all connections in graph
	while (IsGraphFull() == false){
		AddRandomConnection();
	}
}






//***************************************************************************//
//------- The following functions build the rooms and save the files --------//
//--------------------------- all written by me -----------------------------//
//***************************************************************************//

/*******************************************************************************
 * makeRooms
 * Creates the 10 potential rooms for the adventure program using the names
 * available in roomNames.  
 * Once done the 10 available rooms can be accessed via the rooms array
 * ****************************************************************************/
void makeRooms(void){
	//printf("In makeRooms\n");
	// make 10 rooms
	int i;
	for (i = 0; i < 10; i++){
		//printf("room %d: %s\n", i, roomNames[i]);
		// allocate the memory for a Room
		rooms[i] = calloc(1, sizeof(struct Room));
		
		// allocate the memory for the rooms name
		rooms[i]->name = calloc(15, sizeof(char));
		
		// copy the rooms name in
		strcpy(rooms[i]->name, roomNames[i]);
		
		// allocate the memory for the room type
		// will set after the room order is randomized
		rooms[i]->roomType = calloc(11, sizeof(char));
		
		// set initial outbound connections
		rooms[i]->numOutboundConnections = 0;
	}
	//printRooms(10);
}



/*******************************************************************************
 * ShuffleRooms
 * Takes the 10 available rooms in the rooms array and randomly shuffles them
 * in place using pointer swaps. Based loosly on the Fisher-Yates shuffle
 * Ref: https://en.wikipedia.org/wiki/Fisher-Yates_shuffle.
 * ****************************************************************************/
void shuffleRooms(void){
	//printf("In shuffleRooms\n");

	int i;                // array position
	int j;                // swap position
	struct Room* temp;    // holds struct Room to swap

	// every room has a chance to be swapped
	for ( i = 0; i < 10; i++){
		// generate a random j value (0-9)
		j = rand() % 10;

		// swap the pointers
		temp = rooms[i];
		rooms[i] = rooms[j];
		rooms[j] = temp;	
	}
	//printRooms();
}



/*******************************************************************************
 * typeRooms
 * adds the room type to the first 7 Room structs in the rooms array
 * ****************************************************************************/
void typeRooms(void){
	//printf("In typeRooms\n");

	// string copy because we cant just assign strings
	// we need a start room and a end room
	strcpy(rooms[0]->roomType, "START_ROOM");
	strcpy(rooms[1]->roomType, "END_ROOM");

	// and the rest are mid rooms
	int i;
	for (i = 2; i < 7; i++){
		strcpy(rooms[i]->roomType, "MID_ROOM");
	}
}



/*******************************************************************************
 * makeDirectory
 * Makes the directory where the room files will be saved.
 * ****************************************************************************/
void makeDirectory(void){
	//printf("In makeDirectory");
	char* dirName = calloc(20, sizeof(char));  // directory to save files
	
	// get the process id of this program
	// ref: https://stackoverflow.com/questions/15262315/how-to-convert-pid-t-to-string
	char processID[6];
	memset(processID, 0, 6);
	snprintf(processID, 6, "%d", (int)getpid());

	char dirNamePre[] = "howellp.rooms.";   // directory name prefix

	// combine to make the directory name
	strcpy(dirName, dirNamePre);
	strcat(dirName, processID);
	//printf("The directory will be called: %s\n", dirName);

	// create directory to save files into. Returns 0 on success	
	int badDir = mkdir(dirName, 0755);
	if (badDir)
		printf("Error creating save directory.");
	
	// set current woriking directory so we can write files
	chdir(dirName);

	// free ze memory!
	free(dirName);
}



/*******************************************************************************
 * fileRooms
 * Now that the first 7 rooms in the rooms array are connected we can write each
 * room to a file for use with our adventure program. Each room will be in its 
 * own file in the howellp.rooms.### directory, created in the makeDirectory
 * function.
 *
 * ****************************************************************************/
void fileRooms(void){
	//printf("In fileRooms");
	FILE* currFile;     // file pointer to current open file
	int i, j;      // tracks 7 files, connections each file has

	for (i = 0; i < 7; i++){
		// create the file and set it for writing
		currFile = fopen(fileNames[i], "w");		
				
		// write the room name
		fprintf(currFile, "ROOM NAME: %s\n", rooms[i]->name);

		// for each connection the current room has
		for (j = 0; j < rooms[i]->numOutboundConnections; j++){
			// write the connection string 
			fprintf(currFile, "CONNECTION %d: %s\n", 
					j+1, rooms[i]->outboundCon[j]->name);

		}
		// write the room type string
		fprintf(currFile, "ROOM TYPE: %s\n", rooms[i]->roomType);

		// close the current file
		fclose(currFile);
	}
}



/*******************************************************************************
 * freeRooms
 * loops through the rooms array and for each Room struct frees the allocated
 * memory for the struct name, type, and the struct itself.
 *
 * ****************************************************************************/
 void freeRooms(void){
	//printf("In freeRooms\n");
	int i;
	for (i = 0; i < 10; i++){
		free(rooms[i]->name);
		free(rooms[i]->roomType);
		free(rooms[i]);
	}
}



/*******************************************************************************
 * main
 * controls the main flow of the program. We start by creating all of the rooms
 * and then shuffling them (their pointers) in place. After getting a random 
 * order we give each room a type. Then each room is given between 3-6 mutual 
 * connections to other rooms.
 * Once the rooms are ready, we create a directory to write them to. We then 
 * the 7 rooms to their own file within the created directory. With the files
 * saved we finally free the Room structs and all member variable memory.
 *
 * ****************************************************************************/
int main(){	
	// seed the random number generator
	srand(time(NULL));

	makeRooms();
	shuffleRooms();
	typeRooms();
	connectRooms();
	//printRooms(7);
	makeDirectory();
	fileRooms();
	freeRooms();

	return (0);
}












