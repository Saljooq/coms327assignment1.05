//#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
char* name = "Altaf";
int print_end(int success)
{
	for (int i = 0; i < 80; i++){
		for (int j = 0; j< 20; j++){
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
void init_terminal()
{
	initscr();
	raw();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);

}
void render_board(int j)
{
	// int key = getch();
	// mvaddch(j,10,key);
	// printf("%d\n", key);

	for (int i = 0; i < 5; i++)
	{
		usleep(200000);
		clear();
		mvaddch(0,i,name[i]);

		refresh();
	}

}
int main(){
//printf("Hello World!\n");
//for (int i = 0; i < 5; i++)
//{
//	chtype ch = getch();
//	mvaddch(i, 1, ch);
//}
init_terminal();
for (int i = 0; i < 8; i++) {
	render_board(i);
	//usleep(1000000);
}

print_end(0);
//usleep(500000);
endwin();
}
