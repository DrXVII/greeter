//c std lib
#include <stdio.h> //printf(), scanf(), fopen(), etc
#include <stdlib.h>
#include <string.h> //strlen(), strerror()
#include <time.h> //time(), localtime()
#include <errno.h>

//linux
#include <ncurses.h>

#define MAX_CMD_LEN 1024

int init_nc(void); //initialise ncurses
int view_file(const char *_fpath); //function for viewing files
int edit_file(const char *_fpath); //function for editing files
int slow_print_file(const char *_fpath); //prints file slowly char by char
int print_mid(int _y, const char* _s); //print in middle of screen (on x axis)
int zeroize_str(char *_str, unsigned _len); //fill a char array with null bytes
int get_strings(int *strc_, char **strv_, const char *_fname); //gets strings from config file
int main_menu(void);

int main(void)
{
    init_nc();

    main_menu();

    endwin(); //end ncurses mode
    return 0;
}

int init_nc(void)
{
    initscr();
    cbreak(); //get raw char input except spec chars like CTRL-C, CTRL-Z, etc.
    noecho(); //do not echo input to screen (to stdout that is i believe)
    keypad(stdscr, true); //enable function keys (like F1, F2, pg_up, etc...)
    curs_set(0); //make the cursor invisible

    if(!has_colors()) {
        mvprintw(0, 0, "This terminal does not support color!\n");
        getch();
    }
    else { start_color(); }

    //initialising color pairs
    init_pair(1, COLOR_BLACK, COLOR_WHITE);

    return 0;
}

int view_file(const char *_fpath)
{
    const char program[] = "vim -R ";
    char command[MAX_CMD_LEN];

    zeroize_str(command, sizeof(command));

    //construct the command string
    strcat(command, program);
    strcat(command, _fpath);

    endwin(); //exit ncurses just in case
    system(command);
    initscr(); //reinitialise ncurses

    return 0;
}

int edit_file(const char *_fpath)
{
    const char program[] = "vim ";
    char command[MAX_CMD_LEN];

    zeroize_str(command, sizeof(command));

    //construct the command string
    strcat(command, program);
    strcat(command, _fpath);

    endwin(); //exit ncurses just in case
    system(command);
    initscr(); //reinitialise ncurses

    return 0;
}

int slow_print_file(const char *_fpath)
{
    FILE *file;
    file = fopen(_fpath, "r");
    int err = errno;

    move(0,0);
    if(file) {
        int ch = 0;
        int cmd = 0;

        clear();

        halfdelay(1); //getch() will wait a bit and then continue
        while((ch = getc(file)) != EOF) {
            addch(ch);
            if(ch == '*'
            || ch == ' '
            || ch == '+'
            || ch == '-'
            || ch == '='
            || ch == ','
            || ch == '.') {
                continue;
            }

            cmd = getch();
            if(cmd != ERR) { break; }
        }
        cbreak(); //getch() will wait for input indefinitely
    }
    else {
        mvprintw(0, 0, "ERROR(%d - %s): could not read %s\n",
                err, strerror(errno), _fpath);
    }

    int max_y;
    int max_x; //only needed fort the getmaxyx
    getmaxyx(stdscr, max_y, max_x);

    mvprintw(max_y - 1, 0, "press any key to continue");
    getch(); //press any key to continue
    clear();

    fclose(file);

    return 0;
}

int print_mid(int _y, const char* _s)
{
    int max_x = getmaxx(stdscr);
    int mid_x = max_x / 2;
    int our_x = mid_x - (strlen(_s) / 2);
    mvprintw(_y, our_x, _s);
    return our_x;
}

int main_menu(void)
{
    int cmd = 0;

    const char cfg_fname[] = "config.cfg";
    int strc;
    char **strv;

    //get_strings(&strc, strv, cfg_fname);

    //TODO read from config instead of hardcoding, if no config found, read defaults.cfg
    char daily_fpath[] = "/home/egidijus/Dropbox/suitcase/become_better/get_better.txt";
    char todo_fpath[] = "$HOME/Dropbox/suitcase/todo";

    enum Opts {daily = 0, todo, quit};
    int default_sel = daily;
    int last_sel = quit;

    int opt_count = last_sel + 1;
    char *entries[opt_count]; 

    const char menu_header[] = "********** MAIN MENU **********";
    const char menu_footer[] = "*******************************";
    const int menu_w = strlen(menu_header);

    entries[daily] = "view daily tips";
    entries[todo] = "edit todo list";
    entries[quit] = "quit to terminal";

    char date_str[64];
    const int menu_min_y = 0;
    const int opts_start_pos = 2;
    int sel = default_sel;
    while(cmd != 'q') {
        //TODO no need to clear the entire screen
        //BUG flicker on some (very fast?) terminals - should be resolved by above todo

        time_t t = time(NULL);
        struct tm *tm_st = localtime(&t);

        strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S %A", tm_st);

        print_mid(menu_min_y, date_str);
        int menu_min_x = print_mid(menu_min_y + 1, menu_header);

        for(unsigned i = 0; i < opt_count; ++i) {
            if(i == sel) { //paint selected menu option differently
                attron(COLOR_PAIR(1));
            }
            mvprintw(i + opts_start_pos + menu_min_y, menu_min_x + 2, entries[i]);
            attroff(COLOR_PAIR(1)); //returning to default color pair

            mvprintw(i + opts_start_pos + menu_min_y, menu_min_x,
                    "*");
            mvprintw(i + opts_start_pos + menu_min_y, menu_min_x + menu_w - 1,
                    "*");
        }
        mvprintw(sel + opts_start_pos + menu_min_y, menu_min_x,
                ">");

        mvprintw(opt_count + opts_start_pos + menu_min_y, menu_min_x,
                menu_footer);

        halfdelay(10); //wait a sec (10/10 of a second) for input
        cmd = getch();
        cbreak(); //turn of input wait limit

        if(cmd == ERR) { continue; }

        if(cmd == 'k' && sel > 0) { --sel; }
        else if(cmd == 'j' && sel < opt_count - 1) { ++sel; }

        if(cmd != 'l') { continue; }

        switch(sel) {
            case daily: slow_print_file(daily_fpath); break;
            case todo: edit_file(todo_fpath); break;
            case quit: cmd = 'q'; break;
            default: sel == default_sel; break;
        }
    }

    return 0;
}

int zeroize_str(char *_str, unsigned _len)
{
    for(unsigned i = 0; i < _len; ++i) {
        _str[i] = '\0';
    }

    return 0;
}

//TODO work in progress
int get_strings(int *strc_, char **strv_, const char *_fname)
{
    char buf[1024];
    zeroize_str(buf, sizeof(buf));
    FILE *file;

    file = fopen("main.cfg", "r");

    *strc_ = 0;
    while(1) {
        fgets(buf, sizeof(buf) - 1, file);
        if(feof(file)) { break; }

        //TODO allocate memory, deallocate when no longer needed
        strcpy(strv_[*strc_], buf);
        ++(*strc_);
    }

    return 0;
}
