//#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <endian.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <unistd.h>

#define MAX_ROOMS 12
#define MIN_ROOMS 6

#define xlenMax 78
#define ylenMax 19


#define minRoomxlen 4
#define minRoomylen 3

#define maxStairs 3
#define minStairs 1

#define BIT_SMART 0x1
#define TELE 0x2
#define TUN 0x4
#define ERAT 0x8

/*the room is a struct to save the relevant coordinates and leghts of a room*/
typedef struct room
{
	int xloc;
	int yloc;
	int xlen;
	int ylen;
}room;
/*node - this contains x and y coordinates and address of next and prev node;*/
typedef struct node {
	int xcoor;
	int ycoor;
	struct node* prev;
	struct node* next;
} node;
/*node_heap - this contains the head node address (last node entered), the tail node address (first node entered) and the size of the node_heap*/
typedef struct node_heap {
	node* head;
	node* tail;
	int size;
} node_heap;
/*neighbourhood - this contains an 8x2 array that contains all possible neighour's coordinates (x,y) and the size - num of neighours*/
typedef struct neighbourhood{
int store[8][2];
int size;
}neighbourhood;
/*This struct will contain all the data we will need for pc*/
typedef struct PC{
int speed;
int x;
int y;
}PC;
/*ALL non-pc players data will be stored here*/
typedef struct NPC{
uint8_t character;
int x;
int y;
int speed;
int ifPCseen;
int PCx;
int PCy;
}NPC;
/*These nodes store all the players and their data needed to put them on the heap*/
typedef struct player_node{
int ifPC;
PC* pc;
NPC* npc;
int next_turn;
int when_initiated;
int alive;
struct player_node* prev;
struct player_node* next;

}player_node;
/*This merely contains the head and tail player_node of the heap. It is used to access all the players*/
typedef struct player_node_heap {
	player_node* head;
	player_node* tail;
	//int size;
} player_node_heap;

int getkey(int x,int y, int *nextx, int *nexty);

void init_terminal();
int print_end(int success);
int populate_heap(player_node_heap** h);//This puts all the player_node in a heap
int push_player_node(player_node_heap* h, player_node* p);//used to add players to heap
int kill_all();//This is used to make sure all the players malloc'd get freed
int initialize_pc(PC** p);//this is used to take a pc struct's pointer and create a node and place it in random spot.
int initialize_players(int n, PC* p);//used to create all the npc's as per the specs.
/*These are all the prototypes that we'll use later*/
int getNeighbour(int x, int y, neighbourhood* n);
int push(node_heap* nh, int x, int y);
int pop(node_heap* nh, int* x, int* y);
int print_difficulty(int PCposx, int PCposy, int diff_print[xlenMax][ylenMax]);


int makes_sense(room rooms[], int numRooms);
int not_so_rand_roomsize_resizer(int numRooms);
int print_dungeon();
int djik (int xcoordinate, int ycoordinate, int ifdigger);
int print_hardness();
int print_neighbour_movement(int PCposx, int PCposy, int hardness[xlenMax][ylenMax]);

int next_move(player_node *pn, PC* pc, int* ifend, player_node_heap* h);
int pop_player(player_node_heap* nh, int* ifend, player_node** p);
int if_in_room(PC* pc);//updates all bots that share same room as pc

uint8_t player_init_counter = 0;
int distant_from_pc(PC* p, int x, int y);//creates a giant square centered at PC to make sure no bots are initialised too close

/*the five grids are being saved so all the methods will have access to them*/
char grid[xlenMax][ylenMax];
int hardness[xlenMax][ylenMax];
int difficulty[xlenMax][ylenMax];//this will be used to save data for distance of non-tunnelers
int difficulty_t[xlenMax][ylenMax];//this is to save data for tunnelers
uint8_t shortPathRecord[xlenMax][ylenMax];//this keeps record of what is on the queue and what isn't for djik* algo

player_node *grid_players[xlenMax][ylenMax];//this will be used to store pointers of all the player nodes

int main(int argc, char* argv[])
{

	int i, j, x, y, k; //we declare most of the common variables we'll be using late
	//the grid below will be use to store all the characters for dungeon
	//char grid[xlenMax][ylenMax];
	//int hardness[xlenMax][ylenMax];
	int numRooms, numUpstairs, numDownstairs;
	uint8_t xPCpos, yPCpos;



	room *rooms;

	//first we populate the grid with spaces
	for (i = 0; i < xlenMax; i++)
	{
		for (j = 0; j < ylenMax; j++)
		{
			grid[i][j] = ' ';
			grid_players[i][j]=NULL;
		}
	}

	//load method goes ther
	for (i = 1; i < argc; i++)
	{
			if (!(strcmp(argv[i], "--load")))
			{
				j = 1;
				break;
			}

	}

	if (j==1)
	{
		//printf("load found\n");

		FILE *f;

		char *home = getenv("HOME");
		char *gamedir = ".rlg327";
		char *savefile = "dungeon";
		char *path = malloc(strlen(home) + strlen(gamedir) + strlen(savefile) + 2 + 1);
		sprintf(path, "%s/%s/%s", home, gamedir, savefile);

		if( !( f = fopen( path, "r"))) {/*printf("Failed to open file\n");*/ return 1;}
		free(path);


		uint8_t temp8;
		uint16_t temp16;
		uint32_t temp32;

		char filetype[12];
		fread(filetype, sizeof(char), 12, f);

		//dealing with version
		fread(&temp32, sizeof(temp32), 1, f);

		//dealing with filesize - will need to adjust endian if to be used
		fread(&temp32, sizeof(temp32), 1, f);

		//dealing xPCpos and yPCpos
		fread(&xPCpos, sizeof(uint8_t), 1, f);
		fread(&yPCpos, sizeof(uint8_t), 1, f);

		//now we populate the dungeon matrix
		for (j = 0; j < xlenMax + 2; j++)
		{
			fread(&temp8, sizeof(uint8_t), 1, f);
		}

		for (i = 0; i < ylenMax; i++)
		{

			fread(&temp8, sizeof(uint8_t), 1, f);

			for (j = 0; j < xlenMax; j++)
			{
				fread(&temp8, sizeof(uint8_t), 1, f);
				hardness[j][i] = temp8;
			}

			fread(&temp8, sizeof(uint8_t), 1, f);

		}

		for (j = 0; j < xlenMax + 2; j++)
		{
			fread(&temp8, sizeof(uint8_t), 1, f);
		}
		//end of hardness reading

		//number of Rooms are entered here
		fread(&temp16, sizeof(uint16_t), 1, f);
		numRooms = be16toh(temp16);

		//next we fill in the matrix rooms
		rooms = malloc(numRooms * sizeof(room));

		//here we write the coordinates of the room
		for (i = 0; i < numRooms; i++)
		{
			fread(&temp8, sizeof(uint8_t), 1, f);
			rooms[i].xloc = temp8 - 1;

			fread(&temp8, sizeof(uint8_t), 1, f);
			rooms[i].yloc = temp8 - 1;

			fread(&temp8, sizeof(uint8_t), 1, f);
			rooms[i].xlen = temp8;

			fread(&temp8, sizeof(uint8_t), 1, f);
			rooms[i].ylen = temp8;

		}

		//next we populate the room area with the downstairs
		for (x = 0; x < numRooms; x++)
		{

			for (i = rooms[x].xloc; i < (rooms[x].xloc + rooms[x].xlen); i++)
			{
				for (j = rooms[x].yloc; j < (rooms[x].yloc + rooms[x].ylen); j++)
				{
					grid[i][j] = '.';
				}
			}
		}

		//next we deal with upstairs
		fread(&temp16, sizeof(uint16_t), 1, f);
		numUpstairs = be16toh(temp16);


		for (i = 0; i < numUpstairs; i++)
		{

			fread(&temp8, sizeof(uint8_t), 1, f);
			x = temp8 - 1;

			fread(&temp8, sizeof(uint8_t), 1, f);
			y = temp8 - 1;
			grid[x][y] = '<';

		}

		//next we deal with downstairs
		fread(&temp16, sizeof(uint16_t), 1, f);
		numDownstairs = be16toh(temp16);


		for (i = 0; i < numDownstairs; i++)
		{

			fread(&temp8, sizeof(uint8_t), 1, f);
			x = temp8 - 1;

			fread(&temp8, sizeof(uint8_t), 1, f);
			y = temp8 - 1;
			grid[x][y] = '>';

		}
		//next we populate the corridors wherever hardness is zero and there is no room or stairs (or in this case wherever there is space in the grid)
		for (i = 0; i < ylenMax; i++)
		{
			for (j = 0; j < xlenMax; j++)
			{
				if (hardness[j][i] == 0)
				{
					if (grid[j][i] == ' ') grid[j][i] = '#';
				}
			}
		}

		fclose(f);
		//printf("load found - final - no error while loading\n");

	}
	//this is where we start processing the dungron if the load was not found
	else
	{

		//we will start out by creating a seed with time-0 to access some randomeness
		srand(time(0));

		//populating the hardness randomly
		for (i = 0; i < ylenMax; i++)
		{
			for (j = 0; j < xlenMax; j++)
			{
				hardness[j][i] = 1 + (rand() % 254);
			}
		}

		numRooms = MIN_ROOMS + (rand() % (MAX_ROOMS - MIN_ROOMS + 1));

		rooms = malloc(numRooms * sizeof(room));

		int resizer = not_so_rand_roomsize_resizer(numRooms);//we use this function to obtain a denominator to limit the size of the rooms

		//the if conditions used to obtain the max length of the room help avoid the floating point exception (core dump) later when we use it with modulus later
		int maxRoomxlen = xlenMax / resizer;
		if (maxRoomxlen <= minRoomxlen) maxRoomxlen = minRoomxlen + 1;


		int maxRoomylen = ylenMax / resizer;
		if (maxRoomylen <= minRoomylen) maxRoomylen = minRoomylen + 1;

		//printf("num Rooms = %d\n", numRooms); //uncomment to see num of rooms generated

		//this loop keeps going till random coordinates and lengths are obtained from random function that make sense
		while (1)
		{
			for (i = 0; i < numRooms; i++)
			{
				rooms[i].xloc = rand() % xlenMax;
				rooms[i].yloc = rand() % ylenMax;
				rooms[i].xlen = minRoomxlen + rand() % ((maxRoomxlen) - minRoomxlen);
				rooms[i].ylen = minRoomylen + rand() % ((maxRoomylen) - minRoomylen);
			}
			if (makes_sense(rooms, numRooms)) break;
		}


		//Next we populate the grid with '.' as per the randomised coordinates that made sense that we obtained earlier
		for (x = 0; x < numRooms; x++)
		{

			for (i = rooms[x].xloc; i < (rooms[x].xloc + rooms[x].xlen); i++)
			{
				for (j = rooms[x].yloc; j < (rooms[x].yloc + rooms[x].ylen); j++)
				{
					grid[i][j] = '.';
					hardness[i][j] = 0;
				}
			}
		}

		//next we carve out a path between adjacent rooms in which we use the former's x coordinate and latter's y-coordinates to create a mid-point
		for (int x = 0; x < numRooms - 1; x++)
		{
			int middlex = rooms[x].xloc;
			int middley = rooms[x + 1].yloc;
			int i;//i will save the direction of the path

			if (rooms[x].yloc > middley) i = 1;
			else i = -1;

			//first we go from from midpoint to former room
			for ( j = middley; j != rooms[x].yloc; j += i)
			{
				if (grid[middlex][j] != '.')
				{
					grid[middlex][j] = '#';
					hardness[middlex][j] = 0;
				}
			}

			//then we go from midpoint to latter room
			if (rooms[x + 1].xloc > middlex) i = 1;
			else i = -1;

			for ( j = middlex; j != rooms[x + 1].xloc; j += i)
			{
				if (grid[j][middley] != '.')
				{
					grid[j][middley] = '#';
					hardness[j][middley] = 0;
				}
			}

		}


		//here we randomise the upwards and downward staircases and insert them wherever the random coordinates and its horizontal neighbours are part of room
		for (i = 0; i < 2; i++)
		{
			//first iteration adds random number of '<' to the grid, second adds '<'
			char staircase;
			if (i == 0) staircase = '<';
			else staircase = '>';

			int numStairs = minStairs + rand() % ((maxStairs) - minStairs);
			if (i == 0)
			{
				staircase = '<';
				numUpstairs = numStairs;

			}
			else
			{
				staircase = '>';
				numDownstairs = numStairs;
			}

			for (j = 0; j < numStairs; j++)
			{
				//while loops below keeps going till a successfuk coordinate is found
				while (1)
				{
					x = 1 + (rand() % (xlenMax - 2));//this ensures that we're not on the left or the right edge because the condition below checks horizontal neighbours
					y = (rand() % (ylenMax));

					if (grid[x][y] == '.' && grid[x - 1][y] == '.' && grid[x + 1][y] == '.')
					{
						grid[x][y] = staircase;
						break;
					}
				}


			}
		}


		//this is where we try to position PC on the floor, making sure there's floor there
		xPCpos = 0;
		yPCpos = 0;
		for (i = 0; i < ylenMax; i++)
		{
			k = 0;
			for (j = 0; j < xlenMax; j++)
			{
				if (grid[j][i] == '.')
				{
					xPCpos = j+1;
					yPCpos = i+1;
					k=1;
					break;
				}

			}
			if (k) break;
		}

	}
	//this is where processing of the dungeon ends

	/*the method below will help produce the desired result for 1.04*/

	//printf("\nPC is at (y, x): %d, %d\n\n", yPCpos, xPCpos);
	//below is where we print out the actual grid
	//print_dungeon();


	//here we get the argument for the number of monsters
	j = 0;
	for (i = 1; i < argc; i++)
	{
			if (!(strcmp(argv[i], "--nummon")))
			{
				j = 1;
				break;
			}

	}

	if (j) j = atoi(argv[i+1]);
	else j = 10;
	//processing for nummon tags beings here


	//printf("\n\n\n");
	PC* pc;
	initialize_pc(&pc);//this is where pc gets initialised
	//printf("\nPC has been initialised; the coordinates accessible from main are %d, %d\n", pc->x, pc->y);
	initialize_players(j, pc);//this initialises all the bots
	player_node_heap* h;
	populate_heap(&h);//all monsters and pc get loaded on a heap

	init_terminal();
	print_dungeon();
	i = 0;
	player_node* curr = NULL;
	while (!(i))
	{
		pop_player(h, &i, &curr);//this is where a node 'curr' gets selected to make next move
		if (!(i))//if its not the case that theres only one player left, then game goes on or else pc wins
		{
			next_move(curr, pc ,&i, h);//move prints and adds a 0.25s sleep as well


			/*Below the whole heap can be made visible for debugging*/
			// while(curr2!=NULL)
			// player_node* curr2= h->head;
			// {
			// 	printf("%d-%d->",curr2->next_turn, curr2->ifPC);
			// 	curr2 = curr2->next;
			// }
			// printf("\n");
		}

	}
	if (i==2){
		print_dungeon();
		usleep(2000000);
		print_end(0);//printf("\n\n\n\n\n\n\n\n\n\n\n\nPC LOST\n\n\n\n\n\n\n\n");
	}
	else if (i==3)print_end(1);//printf("\n\n\n\n\n\n\n\n\n\n\n\nPC WON\n\n\n\n\n\n\n\n");

	endwin();

	kill_all();
	free(h);


	//Now we check to see if there's a save switch to update the /.rlg327/dungeon
	j = 0;
	for (i = 1; i < argc; i++)
	{
			if (!(strcmp(argv[i], "--save")))
			{
				j = 1;
				break;
			}

	}
	//processing for save tags beings here
	if (j)
	{
		//printf("save found\n");
		FILE *f;

		char *home = getenv("HOME");
		char *gamedir = ".rlg327";
		char *savefile = "dungeon";
		char *path = malloc(strlen(home) + strlen(gamedir) + strlen(savefile) + 2 + 1);
		sprintf(path, "%s/%s/%s", home, gamedir, savefile);

		if( !( f = fopen( path, "w"))) {printf("Failed to open file\n"); return 1;}
		free(path);

		char* marker = "RLG327-S2021";
		fwrite(marker, sizeof(char), 12, f);

		uint32_t version = 0;
		version = htobe32(version);
		fwrite(&version, sizeof(uint32_t), 1, f);

		//calculate the size of the file, meanwhile the size is taken to be zero
		uint32_t size = 1708 + (4 * numRooms) + (2 * (numUpstairs + numDownstairs));
		size = htobe32(size);
		fwrite(&size, sizeof(uint32_t), 1, f);

		//now we enter position of the PC
		fwrite(&xPCpos, sizeof(uint8_t), 1, f);
		fwrite(&yPCpos, sizeof(uint8_t), 1, f);


		//next we write the dungeon matrix - we will have to artificially populate the file with max hardness on border
		uint8_t temp8;

		for (j = 0; j < xlenMax + 2; j++)
		{
			temp8 = 255;
			fwrite(&temp8, sizeof(uint8_t), 1, f);
		}

		for (i = 0; i < ylenMax; i++)
		{
			temp8 = 255;
			fwrite(&temp8, sizeof(uint8_t), 1, f);

			for (j = 0; j < xlenMax; j++)
			{
				temp8 = hardness[j][i];
				fwrite(&temp8, sizeof(uint8_t), 1, f);
			}

			temp8 = 255;
			fwrite(&temp8, sizeof(uint8_t), 1, f);

		}

		for (j = 0; j < xlenMax + 2; j++)
		{
			temp8 = 255;
			fwrite(&temp8, sizeof(uint8_t), 1, f);
		}

		//number of Rooms are entered here
		uint16_t temp16 = numRooms;
		temp16 = htobe16(temp16);

		fwrite(&temp16, sizeof(uint16_t), 1, f);

		//mext we write the coordinates of the room
		for (i = 0; i < numRooms; i++)
		{
			temp8 = 1 + rooms[i].xloc;
			fwrite(&temp8, sizeof(uint8_t), 1, f);

			temp8 = 1 + rooms[i].yloc;
			fwrite(&temp8, sizeof(uint8_t), 1, f);

			temp8 = rooms[i].xlen;
			fwrite(&temp8, sizeof(uint8_t), 1, f);

			temp8 = rooms[i].ylen;
			fwrite(&temp8, sizeof(uint8_t), 1, f);

		}

		//here we process the number of upstairs
		temp16 = numUpstairs;
		temp16 = htobe16(temp16);
		fwrite(&temp16, sizeof(uint16_t), 1, f);

		//here we enter the coordinates of upstairs
		for (j = 0; j < ylenMax; j++)
		{
			for (k = 0; k < xlenMax; k++)
			{
				if (grid[k][j] == '<')
				{
					temp8 = k + 1;
					fwrite(&temp8, sizeof(uint8_t), 1, f);


					temp8 = j + 1;
					fwrite(&temp8, sizeof(uint8_t), 1, f);

				}
			}
		}

		//here we process the number of downstairs
		temp16 = numDownstairs;
		temp16 = htobe16(temp16);
		fwrite(&temp16, sizeof(uint16_t), 1, f);

		//here we enter the coordinates of downstairs
		for (j = 0; j < ylenMax; j++)
		{
			for (k = 0; k < xlenMax; k++)
			{
				if (grid[k][j] == '>')
				{
					temp8 = k + 1;
					fwrite(&temp8, sizeof(uint8_t), 1, f);

					temp8 = j + 1;
					fwrite(&temp8, sizeof(uint8_t), 1, f);

				}
			}
		}


		fclose(f);
	}
	//processing for save ends here

	free(rooms);
	return 0;
}
/*The first is the makes_sense function that takes the array of rooms and number of Rooms as the argument.
It tries to see if the top edge of one room coincides with area occupied with all the other rooms.
This it does in four ways - first it check if the y coordinates of the top edge are inside
the range+1(1 is added to keep a gap of 1) of the other rooms. If a room is coincides on
 the y-coordinates, then we check the x-coordinates. This we do in three ways, by checking the left corner,
 the right corner and the middle. If any of these indicate intersection then the program ends soonafter because
 it 'doesnt make sense' so the random function will pick some other coordinates. This function also makes sure
 the rooms are within the grid we have and don't exceed.*/
int makes_sense(room rooms[], int numRooms)
{

	int checker = 1;//this essentially marks whether the program makes any sense, 0 indicates it doesnn't

	for (int i = 0; i < numRooms; i++)
	{
		for (int j = 0; j < numRooms; j++)
		{
			if (i != j)
			{
				//first it check if the y coordinates of the top edge are inside the range+1(1 is added to keep a gap of 1) of the other rooms. If a room is coincides on the y-coordinates, then we check the x-coordinates
				if(
				rooms[i].yloc >= rooms[j].yloc &&
				rooms[i].yloc <= (rooms[j].yloc + rooms[j].ylen + 1)
				)
				{
					//If a room is coincides on the y-coordinates, then we check the x-coordinates by checking the left corner, the right corner and the middle.
					if(
					(rooms[i].xloc >= rooms[j].xloc &&//this checks left corner
					rooms[i].xloc <= (rooms[j].xloc + rooms[j].xlen + 1)
					) || (
					rooms[i].xloc + rooms[i].xlen >= rooms[j].xloc &&//this checks right corner
					rooms[i].xloc + rooms[i].xlen <= (rooms[j].xloc + rooms[j].xlen + 1)
					) || (
					rooms[i].xloc < rooms[j].xloc &&//this checks middle
					rooms[i].xloc + rooms[i].xlen > (rooms[j].xloc + rooms[j].xlen + 1)
					)
					) checker = 0;
				}


			}

			if (checker == 0) break;//this helps end the program soon if the coordinates don't make sense

		}

		if (//this condition just makes sure all the coordinates will map the rooms in the available grid
			(rooms[i].xloc + rooms[i].xlen > xlenMax - 1) ||
			(rooms[i].yloc + rooms[i].ylen > ylenMax - 1)
			) checker = 0;

		if (checker == 0) break;//this, along with the break above, ensures we swiftly end the program soon if the coordinates don't make sense
	}

	return checker;
}

/*The second function simply creates a number that we use as a denominator for calculating max_size of the rooms.
The reason for this function was because the program took too long to find coordinates that made sense of number
of rooms greater than 8. This restricts the random function a little bit more - hence the not_so_random part of the name.*/
int not_so_rand_roomsize_resizer(int numRooms)
{
	int roomSizer = (numRooms/2) - 1;

	return roomSizer;
}
/* The print_dungeon method below simply takes the x and y coordinates of the PC to make sure the @ is at the right position*/
int print_dungeon()
{
	int i, j, x;//x will serve as  the top offset
	char npc2;
	x=1;
	//for (i = 0; i < xlenMax; i++) {printf("-");}
	//printf("\n");

	for (i = 0; i < ylenMax; i++)
	{
		//printf("|");
		for (j = 0; j < xlenMax; j++)
		{
			mvaddch(i+x,j, grid[j][i]);
			//if (grid_players[j][i]==NULL) printf("%c", grid[j][i]);
			if (grid_players[j][i]==NULL) mvaddch( i+x,j, grid[j][i]);
			else
			{
				if (grid_players[j][i]->ifPC==1) mvaddch(i+x,j, '@');//printf("@");
				else
				{
					//char* store = sprintf("%x",(grid_players[j][i]->npc->character));
					int npc = grid_players[j][i]->npc->character;
					if (npc <= 9){
					npc2=npc+'0';
					mvaddch(i+x,j, npc2);
					}
					else{
						npc2 = npc+'a'-10;
						mvaddch(i+x,j, npc2);
					}
				}
			}
		}
		//printf("\n");
	}
	refresh();

	//for (i = 0; i < xlenMax; i++) {printf("-");}
	//printf("\n\n\n");
}


/*This will be used to calculate the shortest distance for the monsters
shortPathRecord keeps tab of what is inside the queue
node_heap is used to keep all the coordinates in a priority queue
the while loops keep doing till there's nothing left to process
if the distance from current node is shorter than the one the neighbour cell has then it is checked if the neighbour is already on queue. If it isn't then its pushed on the priority queue
The method takes the coordinates of PC as the argument and whether or not the distance that we're finding for is a digger or not*/
int djik (int xcoordinate, int ycoordinate, int ifdigger)
{
	int i, j, k, x, y;


	//next we calculate shortest distance for non-tunnelers
	if(!(ifdigger))
	{
		for (i = 0; i < ylenMax; i++){
			for(j = 0; j < xlenMax; j++) difficulty[j][i] = INT_MAX;
		}
	}
	else
		{
		for (i = 0; i < ylenMax; i++){
			for(j = 0; j < xlenMax; j++) difficulty_t[j][i] = INT_MAX;
		}
	}


	//first we initialise the set of shortPathRecord to zero so nothing is taken to be processed by djik algo
	for (i = 0; i < ylenMax; i++){
		for (j = 0; j < xlenMax; j++)
		{
			shortPathRecord[j][i] = 0;
		}
	}

	node_heap* newH;
	newH = malloc(sizeof(node_heap));
	newH->size = 0;
	newH->tail = NULL;
	newH->head = NULL;

	push (newH, xcoordinate, ycoordinate);
	shortPathRecord [xcoordinate][ycoordinate] = 1;
	difficulty[xcoordinate][ycoordinate] = 0;
	difficulty_t[xcoordinate][ycoordinate] = 0;

	while(newH->size > 0)
	{
		pop(newH, &i, &j);
		shortPathRecord [i][j] = 0;
		neighbourhood currN;
		getNeighbour(i, j, &currN);

		int init_diff;
		if (ifdigger)	init_diff	= difficulty_t[i][j];
		else init_diff = difficulty[i][j];

		int diff_curr_block;
		if (grid[i][j]== ' ') diff_curr_block = 1 + (hardness[i][j]/85);
		else diff_curr_block = 1;

		for (k = 0; k < currN.size; k++)
		{
			x = currN.store[k][0];
			y = currN.store[k][1];

			if (!(ifdigger))
			{
				if (grid[x][y] != ' ')
				{
					//this is where we process the diggers

					if (difficulty[x][y] > (init_diff + diff_curr_block))
					{
						difficulty[x][y] = init_diff + diff_curr_block;
						//check to see if it is already on the processed stack
						if (!shortPathRecord[x][y])
						{
							push(newH, x, y);
							shortPathRecord[x][y] = 1;
						}
					}
			}
			}
			else
			{
				//this is where we process the diggers

				if (difficulty_t[x][y] > (init_diff + diff_curr_block))
				{
					difficulty_t[x][y] = init_diff + diff_curr_block;
					//check to see if it is already on the processed stack
					if (!shortPathRecord[x][y])
					{
						push(newH, x, y);
						shortPathRecord[x][y] = 1;
					}
				}

			}

		}


	}
	free(newH);
	return 0;
}
/*This looks up the coordinates of all the cells around the targeted one. The coordinates of the targeted cell and the neighbourgood that we need to populate*/
int getNeighbour(int x, int y, neighbourhood* n){
	int i,j;
	int size = 0;
	int store[8][2]; //= n->store;
	//we start from the left and the move in the clockwise fashion
	if (x > 0) {
		store[size][0] = x - 1;
		store[size][1] = y;
		size++;
	}
	if ( (x > 0) && (y > 0) ){
		store[size][0] = x - 1;
		store[size][1] = y - 1;
		size++;
	}
	if (y > 0) {
		store[size][0] = x;
		store[size][1] = y - 1;
		size++;
	}
	if ( (x < (xlenMax - 1)) && (y > 0) ){
		store[size][0] = x + 1;
		store[size][1] = y - 1;
		size++;
	}
	if (x < (xlenMax - 1)) {
		store[size][0] = x + 1;
		store[size][1] = y;
		size++;
	}
	if ( (x < (xlenMax - 1)) && (y < (ylenMax - 1)) ){
		store[size][0] = x + 1;
		store[size][1] = y + 1;
		size++;
	}
	if (y < (ylenMax - 1)) {
		store[size][0] = x;
		store[size][1] = y + 1;
		size++;
	}
	if ( (x > 0) && (y < (ylenMax - 1)) ){
		store[size][0] = x - 1;
		store[size][1] = y + 1;
		size++;
	}

//finally getting the copy in neighbourhood
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < size; j++){
			n->store[j][i] = store[j][i];
		}
	}
	n->size = size;

	return 0;
}
/*this method takes the coordinates of the cell and the heap that we're suppose to push them into. A new node will be created and pushed into being the head. If the size is zero, the head and tail will be the newNode. If it isnt, then its simply the head*/
int push(node_heap* nh, int x, int y)
{
	node *newNode = malloc(sizeof(node));
	newNode->xcoor = x;
	newNode->ycoor = y;
	newNode->next = NULL;

	if (!(nh->size)){
		newNode->prev = NULL;
		nh->tail = newNode;
		nh->head = newNode;
		nh->size++;
	}
	else{
		nh->head->next = newNode;
		newNode->prev = (nh->head);
		nh->head = newNode;
		nh->size++;
	}

	return 0;

}
/*This is used to save the results of the node on the tail in the integer addresses provided and then free the node in the tail*/
int pop(node_heap* nh, int* x, int* y)
{
	if (nh->size == 1)
	{
		*x = nh->tail->xcoor;
		*y = nh->tail->ycoor;
		free(nh->tail);
	}
	else
	{
		node* temp;
		*x = nh->tail->xcoor;
		*y = nh->tail->ycoor;
		temp = nh->tail;
		nh->tail = nh->tail->next;
		nh->tail->prev = NULL;
		free(temp);

	}
	nh->size -= 1;
	return 0;

}

/*This method is used to make sure that monsters initialise away from PC and creates a 9x9 square centered at the pc where no npc can intialise*/
int distant_from_pc(PC* p, int x, int y){
	int PCx,PCy;
	//printf("we're in distant method");
	PCx = p->x;
	PCy=p->y;
	//printf("PC is at (%d,%d)", PCx,PCy);
	if ((x > (PCx+4))||(x < (PCx-4))) return 1;
	if ((y > (PCy+4))||(y < (PCy-4))) return 1;
	return 0;
}
/*this is used to take a pc struct's pointer and create a node and place it in random spot.*/
int initialize_pc(PC** pc)
{
	(*pc) = malloc(sizeof(PC));
	int i, j, k;
	i = 1;
	while (i)
	{
		k = rand()%xlenMax;
		j = rand()%ylenMax;

		if (grid[k][j] == '.') i = 0;


	}

	(*pc)->x = k;
	(*pc)->y = j;

	(*pc)->speed = 10;


	player_node* pn = malloc(sizeof(player_node));
	pn->ifPC = 1;
	pn->alive = 1;
	pn->pc = (*pc);
	pn->next_turn = 0;
	pn->when_initiated = player_init_counter++;
	//printf("\nx: %d y: %d\n", k,j);
	grid_players[k][j] = pn;
}
/* This is used to create all the npc's as per the specs. There is a distance of atleast 4 from PC*/
int initialize_players(int n, PC* p)
{
	int i, j, k , t;


	for (t = 0; t < n; t++)
	{

		NPC *npc = malloc(sizeof(NPC));

		//printf("good so far\n");

		i= 1;
		while (i)
		{

			k = rand()%xlenMax;
			j = rand()%ylenMax;

			//printf("---random find:: x is %d and y is %d\n", k,j);

			if (grid[k][j] == '.' && grid_players[k][j]==NULL && distant_from_pc(p, k, j)) i = 0;


		}

		//printf("here! after initialisation");

		npc->x = k;
		npc->y = j;


		//printf("x is %d and y is %d\n", k,j);

		npc->character = rand()&0xf;//any character netween 0-15
		npc->speed = 5+ (rand()&0xf);//speed randomly gets selected from 5-20
		npc-> ifPCseen = 0;


		player_node* pn = malloc(sizeof(player_node));
		pn->ifPC = 0;
		pn->alive = 1;
		pn->npc = npc;
		pn->next_turn = 0;
		pn->when_initiated = player_init_counter++;
		//printf("x: %d y: %d\n", k,j);
		grid_players[k][j] = pn;

	}


}
/*This is a clean-up mechanism that makes sure all remaining players(malloc'd) are freed.*/
int kill_all()
{
	int i, j;
	for (i = 0; i < ylenMax; i++)
	{
		for (j = 0; j < xlenMax; j++)
		{
			if (grid_players[j][i]!=NULL)
			{
				if (grid_players[j][i]->ifPC)
				{
					int tempx = (grid_players[j][i]->pc->x);
					int tempy = (grid_players[j][i]->pc->y);
					free(grid_players[j][i]->pc);
					free(grid_players[j][i]);
					grid_players[tempx][tempy] = NULL;
				}
				else
				{
					int tempx = grid_players[j][i]->npc->x;
					int tempy = grid_players[j][i]->npc->y;
					free(grid_players[j][i]->npc);
					free(grid_players[j][i]);
					grid_players[tempx][tempy] = NULL;
				}
			}
		}
	}

}

/*populates our heap with all the players in the grid_players using the player_push method*/
int populate_heap(player_node_heap** h)
{
	(*h) = malloc(sizeof(player_node_heap));
	(*h)->head = NULL;
	(*h)->tail = NULL;
	for (int i = 0; i < ylenMax; i++)
	{
		for (int j = 0; j < xlenMax; j++)
		{
			if(grid_players[j][i]!=NULL) push_player_node((*h), grid_players[j][i]);
		}
	}
}
/*This is used to push all the new nodes into the heap*/
int push_player_node(player_node_heap* h, player_node* p)
{
	if (h->head==NULL)//nothing in the heap
	{
		h->head = p;
		h->tail = p;
		p->prev = NULL;
		p->next = NULL;
		return 0;
	}
	else
	{
		h->tail->next = p;
		p->prev = h->tail;
		h->tail = p;
		p->next = NULL;
		return 0;
	}

}

/*This is used to select a node in the heap based on the specs (lowest -> next_turn, when_initiated) */
int pop_player(player_node_heap* nh, int* ifend, player_node** output)
{
	player_node* p = nh->head;
	if(p==NULL)
	{
		*ifend = 1;
		return 0;
	}
	if(p->next==NULL)
	{
		*ifend = 3;
		return 0;
	}
	int min_turn = p->next_turn;
	int min_when_initiated = p->when_initiated;
	player_node* min_node = p;
	p = p->next;
	while(p!=NULL)
	{
		if ((p->next_turn) < min_turn)
		{
			min_node = p;
			min_when_initiated = p->when_initiated;
			min_turn = p->next_turn;
		}
		else if((p->next_turn) == min_turn)
		{
			if ((p->when_initiated) < min_when_initiated)
			{
				min_node = p;
				min_when_initiated = p->when_initiated;
			}
		}
		p = p->next;
	}
	//if p->ifpc then print dungeon
	//next_move(min_node, ifend);
	*output = min_node;
	return 0;

}

/*This is used to kill a player, remove it from the heap and free the malloc'd node*/
int kill_player(player_node* p, player_node_heap* h)
{
	//player_node* previous =
	if ((p->prev)!=NULL)
	{
		if (p->next==NULL) p->prev->next = NULL;
		else p->prev->next = p->next;
	}
	else //this is a head
	{
		if (p->next==NULL)
		h->head = NULL;
		else h->head = p->next;
	}
	if ((p->next)!=NULL)
	{
		p->next->prev = p->prev;
	}
	else //this is tail
	{
		if (p->prev==NULL) h->tail =NULL;
		else h->tail = p->prev;
	}

	if (p->ifPC==1)
	{
		int tempx = p->pc->x;
		int tempy = p->pc->y;
		free(p->pc);
		free(p);
		grid_players[tempx][tempy] = NULL;
	}
	else
	{
		int tempx = p->npc->x;
		int tempy = p->npc->y;
		free(p->npc);
		free(p);
		grid_players[tempx][tempy] = NULL;
	}
}

/*This is used to figure out what to do with the node selected with pop_player. nextx, and nexty are set to current location of the bot and will determine where it moves next.
This method first checks if bot is tele or not. If it is tele then ifPCseen gets updated to 1, and PCx and PCy get updated to current PC location.
If not tele, then if_in_room updates to see if tele can see the PC.
Then we check to see if bot it intelligent. If intelligent and if PC is seen- then we will use djik algo (based on PCx and PCy and if digger or not) then update nextx and nexty.
If not intelligent, then if PC is seen, then bot moves the nextx, and nexty, horizontally and then vertically towards the location known to the bot.
If the bot is erratic, then we check random, and there's a 50 percent chance neigbouring cell will be used for nextx or nexty, other 50percent chance, next coordinates remain unchanged.
Then the nextx and nexty is processed - as per the specs. First we check if there's a bot sitting on the next coordinates. If there is (and it isn't the current one) then we kill it and remove it from the heap and the grid_players. If there is no bot- action will depend on whether it can tunnel and what the hardness is.*/
int next_move(player_node *pn, PC* pc, int* ifend, player_node_heap* h)
{

	if (pn->ifPC==1) {
		//printf("\nPC's turn, score of %d \n", pn->next_turn);
		int nextx, nexty;
		getkey(pn->pc->x, pn->pc->y, &nextx, &nexty);

		//boundary check
		if (nextx < 0 || nextx >= xlenMax) nextx = pn->pc->x;
		if (nexty < 0 || nexty >= ylenMax) nexty = pn->pc->y;

		if ( (pn->pc->x!=nextx) || (pn->pc->y!=nexty) )
		{
			if(grid_players[nextx][nexty]!=NULL)
			{
				kill_player(grid_players[nextx][nexty], h);
			}

			grid_players[pn->pc->x][pn->pc->y] = NULL;
			pn->pc->x = nextx;
			pn->pc->y = nexty;
			grid_players[pn->pc->x][pn->pc->y] = pn;

			//wall check
			if (grid[pn->pc->x][pn->pc->y] == ' ')
			{
				grid[pn->pc->x][pn->pc->y] = '#';
				hardness[pn->pc->x][pn->pc->y] = 0;
			}

		}
		pn->next_turn = pn->next_turn + (1000/(pn->pc->speed));
		print_dungeon();

		return 0;

		// getNeighbour(pn->pc->x, pn->pc->y, n);
		// for (int i = 0; i < n->size; i++)
		// {
		// 	if(grid_players[n->store[i][0]][n->store[i][1]]!=NULL)
		// 	{
		// 		kill_player(grid_players[n->store[i][0]][n->store[i][1]], h);
		// 		//player_node* temp = pn;
		// 		grid_players[pn->pc->x][pn->pc->y] = NULL;
		// 		pn->pc->x = n->store[i][0];
		// 		pn->pc->y = n->store[i][1];
		// 		grid_players[n->store[i][0]][n->store[i][1]] = pn;
		// 		break;
		// 	}
		// }
		//
		//
		// pn->next_turn = pn->next_turn + (1000/(pn->pc->speed));
		// usleep(250000);
		// print_dungeon();
		// free(n);
		// //printf("\n\n\n");
		// return 0;

	}

	//printf("\nNPC with a score of %d at x,y: %d,%d named %x, being moved to  ",pn->next_turn, pn->npc->x,pn->npc->y,pn->npc->character);
	//now we've established that the node is not of PC
	int character = pn->npc->character;
	int x = pn->npc->x;
	int y = pn->npc->y;
	neighbourhood* n;
	n = malloc(sizeof(neighbourhood));
	getNeighbour(x, y, n);
	//dijik(x,y);
	int nextx=x;
	int nexty=y;
	int cost = INT_MAX;

	if (character & TELE)
	{
		pn->npc->ifPCseen = 1;
		pn->npc->PCx = pc->x;
		pn->npc->PCy = pc->y;
	}
	else
	{
		if_in_room(pc);//this updates all the ifPCseen and coordinates in all the bots in the room with PC
	}
	if (character & BIT_SMART)
	{
		if (pn->npc->ifPCseen)
		{
			if (character & TUN)
			{
				djik(pn->npc->PCx, pn->npc->PCy, 1);

				for (int i = 0; i < n->size; i++)
				{
					if (difficulty_t[n->store[i][0]][n->store[i][1]] < cost)
					{
						nextx = n->store[i][0];
						nexty = n->store[i][1];
						cost = difficulty_t[n->store[i][0]][n->store[i][1]];
					}
				}


			}else
			{
				djik(pn->npc->PCx, pn->npc->PCy, 0);

				for (int i = 0; i < n->size; i++)
				{
					if (difficulty_t[n->store[i][0]][n->store[i][1]] < cost)
					{
						nextx = n->store[i][0];
						nexty = n->store[i][1];
						cost = difficulty[n->store[i][0]][n->store[i][1]];
					}
				}
			}
		}

	}
	else //dumb ones just move in a straight line towards pc location
	{
		if (pn->npc->ifPCseen)
		{
			if(x > pn->npc->PCx)
			{
				nextx = x - 1;
			}
			else if (x < pn->npc->PCx)
			{
				nextx = x + 1;
			}
			else if (y > pn->npc->PCy)
			{
				nexty = y - 1;
			}
			else if (y < pn->npc->PCy)
			{
				nexty = y + 1;
			}
			//else do nothing
		}
	}

	if (character & ERAT)
	{
		if (rand()& 0x1)
		{
			//printf(" --sorry randomiser activated--  ");
			int selector = rand() % (n->size);
			nextx = n->store[selector][0];
			nexty = n->store[selector][1];
		}
	}

	//printf("%d, %d\n",nextx, nexty);

	//nextx and nexty will be the positions we will use to determine where the monster goes

	if(grid_players[nextx][nexty]==NULL)//no players to kill
	{
		if(character & TUN)
		{
			if(grid[nextx][nexty]==' ' && hardness[nextx][nexty] > 85 )
			{
				hardness[nextx][nexty] -= 85;
			}
			else
			{
				hardness[nextx][nexty] = 0;
				if(grid[nextx][nexty]==' ') grid[nextx][nexty] = '#';
				//player_node* temp = grid_players[x][y];
				grid_players[x][y] = NULL;
				pn->npc->x = nextx;
				pn->npc->y = nexty;
				grid_players[nextx][nexty] = pn;

			}
		}
		else
		{
			if(grid[nextx][nexty]!=' ')
			{
				//player_node* temp = grid_players[x][y];
				grid_players[x][y] = NULL;
				pn->npc->x = nextx;
				pn->npc->y = nexty;
				grid_players[nextx][nexty] = pn;
			}
		}
	}
	else if (nextx!=x || nexty!=y)//we kill someone
	{
		if (grid_players[nextx][nexty]->ifPC==1) *ifend = 2;
		kill_player(grid_players[nextx][nexty], h);
		//player_node* temp = grid_players[x][y];
		grid_players[x][y] = NULL;
		pn->npc->x = nextx;
		pn->npc->y = nexty;
		grid_players[nextx][nexty] = pn;
	}

	pn->next_turn += (1000/(pn->npc->speed));
	free (n);
	return 0;
}
/*This methods identifies all the non-npc players in the room shared by PC, and updated the ifPCseen to PC's current location PCx and PCy for npc*/
int if_in_room(PC* pc)
{
	int upper = pc->y;
	while(upper+1 < ylenMax && grid[pc->x][upper+1]!=' ') upper++;
	int lower = pc->y;
	while(lower > 0 && grid[pc->x][lower-1]!=' ') lower--;
	int right = pc->x;
	while(right+1 < xlenMax && grid[right+1][pc->y]!=' ') right++;
	int left = pc->x;
	while(left > 0 && grid[left-1][pc->y]!=' ') left--;

	for(int i = lower; i <= upper; i++)
	{
		for(int j = left; j <= right; j++)
		{
			if (grid_players[j][i]!=NULL){
				if(!(grid_players[j][i]->ifPC==1)){
					grid_players[j][i]->npc->ifPCseen = 1;
					grid_players[j][i]->npc->PCx = pc->x;
					grid_players[j][i]->npc->PCy = pc->y;
				}
			}
		}
	}

}
/*This will help initialise the screen so we can have see the game, with contant printf's*/
void init_terminal()
{
	initscr();
	raw();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);

}

int print_end(int success)
{
	for (int i = 0; i < ylenMax+1; i++){
		for (int j = 0; j< xlenMax; j++){
			mvaddch(i,j,' ');
		}
	}

	if (success)
	{
		char* win = "YOU WIN!!!!!!!!!!!!!.";
		for (int i = 0; win[i]!='.'; i++) mvaddch(10, 30+i, win[i]);

	}
	else {
		char* lose = "YOU LOSE!!!!!!!!!!!!!.";
		for (int i = 0; lose[i]!='.'; i++) mvaddch(10, 30+i, lose[i]);
	}
	refresh();
	usleep(2000000);
}

int getkey(int prevx,int prevy, int *x,int *y)
{
	chtype ch = getch();
	*x = prevx;
	*y = prevy;
	//mvaddch(*y, *x,' ');
	if (ch=='8') --*y;
	else if (ch=='5');
	else if (ch=='6') ++*x;
	else if (ch=='4')  --*x;
	else if (ch=='2') ++*y;
	else if (ch=='1') {++*y; --*x;}
	else if (ch=='3') {++*y; ++*x;}
	else if (ch=='7') {--*x; --*y;}
	else if (ch=='9') {--*y; ++*x;}
	//else if (ch=='q') endwin();
	//game(y, x);
}
// int getmonsterlist()//this should inherit a linked list of monsters//this should create a while loop that only updates on escape
// {
//
// }
