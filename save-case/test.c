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
int game(int* y, int* x){

	//for (int i = 0; i < 20 ; i++) for (int j = 0; i < 80; j++) mvaddch(i,j,' ');
	mvaddch(*y, *x,'c');
	refresh();
	//mvaddch(*y, *x,' ');
	chtype ch = getch();
	mvaddch(*y, *x,' ');
	if (ch=='8') --*y;
	else if (ch=='5');
	else if (ch=='6') ++*x;
	else if (ch=='4')  --*x;
	else if (ch=='2') ++*y;
	else if (ch=='1') {++*y; --*x;}
	else if (ch=='3') {++*y; ++*x;}
	else if (ch=='7') {--*x; --*y;}
	else if (ch=='9') {--*y; ++*x;}
	else if (ch=='q') endwin();
	game(y, x);
}
void render_board(int j)
{
	chtype key = getch();
	//mvaddch(j,10,key);
	if (key == 27) printf("You PRESSED ESC\n\n");
	else if (key == 260) printf("You PRESSED LEFT\n\n");
	else if (key == 259) printf("You PRESSED TOP\n\n");
	else if (key == 261) printf("You PRESSED RIGHT\n\n");
	else if (key == 258) printf("You PRESSED DOWN\n\n");
	else printf("\n%c\n",key);
	//printf("        %d\n", key);

	// for (int i = 0; i < 5; i++)
	// {
	// 	usleep(200000);
	// 	clear();
	// 	mvaddch(0,i,name[i]);
	//
	// 	refresh();
	// }
	//int y = 10;
	//int x = 20;
	//game(&y, &x);
	refresh();
	//chtype ch = getch();
	//if (ch=='8')

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
	usleep(1000000);
}

print_end(0);
//usleep(500000);
endwin();
}
