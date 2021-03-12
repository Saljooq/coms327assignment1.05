Name: Saljooq Altaf
Net ID: saltaf


Assignment 1.05


For the purpose of this assignment all the printf's were replaced with mvaddch.

There were several helper methods added, the details of which are shown below:

void init_terminal -> This was used to intiate the terminal where the game would play

int getkey -> This method was added to the next_move method, where to determine next move, get key fetches a key and reacts accordingly.

int random_generator -> This essentially regenerates a new dungeon using old references so the game can keep running fluidly

int getmonsterlist -> This generates a rectangle of spaces and displays the distance of the monsters relative to the PC along with their names(characters in hexa)

int print_end -> This is simply use to display msgs like 'You lose' or 'You win' or 'You quit'.

There were no new structs created.
