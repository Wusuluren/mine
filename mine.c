#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <signal.h>
#include <sys/time.h>

#define ROW_SMALL 			10
#define COL_SMALL			10
#define ROW_LARGE 			20
#define COL_LARGE 			20
#define MODE_SMALL			1
#define MODE_LARGE 			2
#define MINE_SMALL 			10
#define MINE_LARGE 			20
#define MINE 				1
#define FLAG 				2
#define PRESSED 			1
#define RELEASED 			2
#define WIN 				1
#define LOSE 				2

#define CHAR_BLANK 			' '
#define CHAR_MINE 			'*'
#define CHAR_FLAG 			'?'
#define CHAR_ROW_SMALL_1 	"------------"
#define CHAR_ROW_SMALL_2    "|          |"
#define CHAR_ROW_LARGE_1    "----------------------"
#define CHAR_ROW_LARGE_2    "|                    |"

int win();
void game_over_splash();
void human_over(int flag);
void clear_cursor();
void draw_cursor();
void move_up();
void move_down();
void move_left();
void move_right();
void play();
int boom();
void set_flag(int state);
void generate_mine();
void select_mode();
void draw_map();
void draw_time();
void draw_flag();
int in_map(int y, int x);
void show_num(int y, int x);
void timer(int sig);
void game_over();
void init();
void init_game();
int set_ticker(int n_msec);

struct weiyi {
	int x, y;
};

int mode;
int flag_cnt, flag_success;
unsigned long cur_time, min_time_small, min_time_large;
int cursor_x, cursor_y;
int old_x, old_y;
int map_small[ROW_SMALL][COL_SMALL];
int map_large[ROW_LARGE][COL_LARGE];
int visited[ROW_LARGE][COL_LARGE];

struct weiyi around[9] = {
	-1,-1, 		0,-1,		1,-1, 
	-1,0, 		0,0, 		1,0, 
	-1,1, 		0,1, 		1,1
};

int main()
{
	init();
	play();
	return 0;
}

void play()
{
	int ch;

	while (1) {
		ch = getch();
		switch (ch) {
			case 'W':
			case 'w':
				move_up();
				break;
			case 'S':
			case 's':
				move_down();
				break;
			case 'A':
			case 'a':
				move_left();
				break;
			case 'D':
			case 'd':
				move_right();
				break;
			case ' ':
				attroff(A_STANDOUT);
				show_num(cursor_y, cursor_x);
				memset(visited, 0, sizeof(int)*ROW_LARGE*COL_LARGE);
				draw_cursor();
				break;
			case 'Z':
			case 'z':
				if ((old_x != cursor_x) || (old_y != cursor_y)) {
					set_flag(PRESSED);
				}
				else {
					set_flag(RELEASED);
					old_x = cursor_x;
					old_y = cursor_y;
				}
				break;
			case 'Q':
			case 'q':
				init_game();
				break;
			default:
				break;
		}
	}
}

int boom()
{
	if (MODE_SMALL == mode) {
		if (MINE == map_small[cursor_y-1][cursor_x-1])
			return 1;
		return 0;
	}
	else if (MODE_LARGE == mode) {
		if (MINE == map_large[cursor_y-1][cursor_x-1])
			return 1;
		return 0;
	}
}

void set_flag(int state)
{
	move(cursor_y, cursor_x);
	if (PRESSED == state) {
		flag_cnt ++;
		addch(CHAR_FLAG);
	}
	else if (RELEASED == state) {
		flag_cnt --;
		addch(CHAR_BLANK);
	}
	refresh();
	draw_flag();

	if (MODE_SMALL == mode) {
		if ((PRESSED == state) && 
				(map_small[cursor_y-1][cursor_x-1]))
			flag_success ++;
		else if ((PRESSED == state) && 
				(map_small[cursor_y-1][cursor_x-1]))
			flag_success --;
	}
	else if (MODE_LARGE == mode) {
		if ((PRESSED == state) && 
				(map_large[cursor_y-1][cursor_x-1]))
			flag_success ++;
		else if ((PRESSED == state) && 
				(map_large[cursor_y-1][cursor_x-1]))
			flag_success --;
	}

	if (win()) {
		human_over(WIN);
	}
}

int in_map(int y, int x)
{
	if (y < 1)
		return 0;
	if (x < 1)
		return 0;
	if (MODE_SMALL == mode) {
		if (y > ROW_SMALL)
			return 0;
		if (x > COL_SMALL)
			return 0;
	}
	else if (MODE_LARGE == mode) {
		if (y > ROW_LARGE)
			return 0;
		if (x > COL_LARGE)
			return 0;
	}

	return 1;
}

int win()
{
	if (MODE_SMALL == mode)
		return ((MINE_SMALL == flag_success) && 
				(MINE_SMALL == flag_cnt));
	else if (MODE_LARGE == mode)
		return ((MINE_LARGE == flag_success) && 
				(MINE_LARGE == flag_cnt));
}

void game_over_splash()
{
	int i, j;

	if (MODE_SMALL == mode) {
		for (i = 0; i < ROW_SMALL; i ++) {
			for (j = 0; j < COL_SMALL; j ++) {
				if (map_small[i][j]) {
					move(i+1, j+1);
					addch(CHAR_MINE);
				}
			}
		}
	}
	else if (MODE_LARGE == mode) {
		for (i = 0; i < ROW_LARGE; i ++) {
			for (j = 0; j < COL_LARGE; j ++) {
				if (map_large[i][j]) {
					move(i+1, j+1);
					addch(CHAR_MINE);
				}
			}
		}
	}
	refresh();
}

void show_num(int y, int x)
{
	int i, cnt = 0;
	int x0, y0;

	if (boom()) {
		game_over_splash();
		human_over(LOSE);
		return;          // note!
	}

	if (win()) {
		human_over(WIN);
	}

	for (i = 0; i < 9; i ++) {
		x0 = x + around[i].x;
		y0 = y + around[i].y;
		if (in_map(y0, x0)) {
		    if ((MODE_SMALL == mode) && 
					(map_small[y0-1][x0-1] == MINE)) {
				cnt ++;
			}
		    else if ((MODE_LARGE == mode) && 
					(map_large[y0-1][x0-1] == MINE)) {
				cnt ++;
			}
		}
	}
	move(y, x);
	addch('0'+cnt);
	refresh();
	visited[y-1][x-1] = 1;

	if (0 == cnt) {
		for (i = 0; i < 9; i ++) {
			x0 = x + around[i].x;
			y0 = y + around[i].y;
			if (in_map(y0, x0) && !visited[y0-1][x0-1]) {
				show_num(y0, x0);
			}
		}
	}
}

void clear_cursor()
{
	char ch;
	move(cursor_y, cursor_x);
	attroff(A_STANDOUT);
	ch = (char)inch();
	addch(ch);
	refresh();
}

void draw_cursor()
{
	char ch;
	move(cursor_y, cursor_x);
	ch = (char)inch();
	attron(A_STANDOUT);
	addch(ch);
	refresh();
}

void move_up()
{
	clear_cursor();
	if (cursor_y > 1)
		cursor_y --;
	draw_cursor();
}

void move_down()
{
	clear_cursor();
	if (MODE_SMALL == mode) {
		if (cursor_y < ROW_SMALL)
			cursor_y ++;
	}
	else if (MODE_LARGE == mode) {
		if (cursor_y < ROW_LARGE)
			cursor_y ++;
	}
	draw_cursor();
}

void move_left()
{
	clear_cursor();
	if (cursor_x > 1)
		cursor_x --;
	draw_cursor();
}

void move_right()
{
	clear_cursor();
	if (MODE_SMALL == mode) {
		if (cursor_x < COL_SMALL)
			cursor_x ++;
	}
	else if (MODE_LARGE == mode) {
		if (cursor_x < COL_LARGE)
			cursor_x ++;
	}
	draw_cursor();
}

void timer(int sig)
{
	static int cnt = 0;

	if (cnt >= 1) {
		cnt = 0;
	cur_time ++;
	draw_time();
	}
	else {
		cnt ++;
	}
}

void draw_flag()
{
	attroff(A_STANDOUT);
	if (MODE_SMALL == mode) {
		move(5, COL_SMALL+3);
		printw("flag number : %d", flag_cnt);
	}
	else if (MODE_LARGE == mode) {
		move(5, COL_LARGE+3);
		printw("flag number : %d", flag_cnt);
	}
	refresh();
	attron(A_STANDOUT);
}

void draw_time()
{
	attroff(A_STANDOUT);
	if (MODE_SMALL == mode) {
		move(0, COL_SMALL+3);
		printw("time : %d hours %d minutes %d seconds", 
				cur_time/3600, cur_time%3600/60, cur_time%3600%60);
		move(2, COL_SMALL+3);
		printw("record : %d hours %d minutes %d seconds", 
				min_time_small/3600, min_time_small%3600/60, 
				min_time_small%3600%60);
	}
	else if (MODE_LARGE == mode) {
		move(0, COL_LARGE+3);
		printw("time : %d hours %d minutes %d seconds", 
				cur_time/3600, cur_time%3600/60, cur_time%3600%60);
		move(2, COL_LARGE+3);
		printw("record : %d hours %d minutes %d seconds", 
				min_time_large/3600, min_time_large%3600/60, 
				min_time_large%3600%60);
	}
	refresh();
	attron(A_STANDOUT);
}

void draw_map()
{
	int i;

	clear();
	if (MODE_SMALL == mode) {
		move(0, 0);
		printw(CHAR_ROW_SMALL_1);
		for (i = 1; i <= ROW_SMALL; i ++) {
			move(i, 0);
			printw(CHAR_ROW_SMALL_2);
		}
		move(i, 0);
		printw(CHAR_ROW_SMALL_1);
	}
	else if (MODE_LARGE == mode) {
		move(0, 0);
		printw(CHAR_ROW_LARGE_1);
		for (i = 1; i <= ROW_LARGE; i ++) {
			move(i, 0);
			printw(CHAR_ROW_LARGE_2);
		}
		move(i, 0);
		printw(CHAR_ROW_LARGE_1);
	}

	refresh();
}

void select_mode()
{
	int ch, flag;

	clear();
	move(0, 10);
	printw("Select Mode");
	move(3, 10);
	printw("1. Small (1)");
	move(5, 10);
	printw("2. Large (2)");
	move(7, 10);
	printw("3. Quit (3)");

	flag = 1;
	while(flag) {
		ch = getch();
		switch (ch) {
			case '1':
				mode = MODE_SMALL;
				flag = 0;
				break;
			case '2':
				mode = MODE_LARGE;
				flag = 0;
				break;
			case '3':
				game_over();
				flag = 0;
				break;
			default:
				break;
		}
	}
}

void human_over(int flag)
{
	set_ticker(0);
	clear_cursor();
	if (MODE_SMALL == mode) {
		move(ROW_SMALL+3, 0);
		if (WIN == flag) {
			if (cur_time < min_time_small) {
				printw("New Time Record : \
						%d hours %d minutes %d seconds", 
						cur_time/3600, 
						cur_time%3600/60, 
						cur_time%3600%60);
				min_time_small = cur_time;
			}
			else {
				printw("You Win!");
			}
		}
		else if (LOSE == flag) {
				printw("You Lose!");
		}
	}
	else if (MODE_LARGE == mode) {
		move(ROW_LARGE+3, 0);
		if (WIN == flag) {
			if (cur_time < min_time_large) {
				printw("New Time Record : \
						%d hours %d minutes %d seconds", 
						cur_time/3600, 
						cur_time%3600/60, 
						cur_time%3600%60);
				min_time_large = cur_time;
			}
			else {
				printw("You Win!");
			}
		}
		else if (LOSE == flag) {
				printw("You Lose!");
		}
	}
	refresh();
	sleep(3);
	init_game();
}

void game_over()
{
	endwin();
	exit(0);
}

void generate_mine()
{
	int i, j, k;

	if (MODE_SMALL == mode) {
		for (k = 0; k < MINE_SMALL; ) {
			i = rand() % ROW_SMALL;
			j = rand() % COL_SMALL;
			if (map_small[i][j] != MINE) {
				map_small[i][j] = MINE;
				k ++;
			}
			else {
				continue;
			}
		}
	}
	else if (MODE_LARGE == mode) {
		for (k = 0; k < MINE_LARGE; ) {
			i = rand() % ROW_LARGE;
			j = rand() % COL_LARGE;
			if (map_large[i][j] != MINE) {
				map_large[i][j] = MINE;
				k ++;
			}
			else {
				continue;
			}
		}
	}
}

void init()
{
	initscr();
	clear();
	cbreak();
	noecho();
	curs_set(0);
	srand(time(0));

	signal(SIGALRM, timer);
	min_time_large = min_time_small = ~0;
	init_game();
}

void init_game()
{
	set_ticker(0);
	attroff(A_STANDOUT);
	select_mode();
	if (MODE_SMALL == mode)
		memset(map_small, 0, sizeof(int)*ROW_SMALL*COL_SMALL);
	else if (MODE_LARGE == mode)
		memset(map_large, 0, sizeof(int)*ROW_LARGE*COL_LARGE);
	draw_map();
	generate_mine();
	cursor_x = (MODE_SMALL == mode) ? COL_SMALL/2+1 : COL_LARGE/2+1;
	cursor_y = (MODE_SMALL == mode) ? ROW_SMALL/2+1 : ROW_LARGE/2+1;
	old_x = cursor_x;
	old_y = cursor_y;
	draw_cursor();
	cur_time = 0;
	flag_cnt = 0;
	flag_success = 0;
	set_ticker(500);
	draw_time();
	draw_flag();
}

int set_ticker(int n_msec)
{
	struct itimerval timeset;
	long n_sec, n_usec;

	n_sec = n_msec / 500;
	n_usec = (n_msec % 500) * 1000L;

	timeset.it_interval.tv_sec = n_sec;
	timeset.it_interval.tv_usec = n_usec;

	timeset.it_value.tv_sec = n_sec;
	timeset.it_value.tv_usec = n_usec;

	return setitimer(ITIMER_REAL, &timeset, NULL);
}
