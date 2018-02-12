//c std lib
#include <stdio.h> //printf(), scanf(), fopen(), etc
#include <stdlib.h>
#include <string.h> //strlen(), strerror()
#include <time.h> //time(), localtime()
#include <errno.h>
#include <assert.h>

//linux
#include <ncurses.h>

//homebrew
#include "config.h"

#define MAX_CMD_LEN 1024
#define MAX_CFG_LN_LEN 1024
#define MAX_MEN_LNS 20
#define VERSION "v1.2.2"

int init_nc(void); //initialise ncurses
int parse_args(int _argc, char **_argv, char **_cfg_fname); //parse arguments passed to program
int view_file(const char *_fpath); //function for viewing files
int edit_file(const char *_fpath); //function for editing files
int slow_print_file(const char *_fpath); //prints file slowly char by char
int print_mid(int _y, const char* _s); //print in middle of screen (on x axis)
int zeroize_str(char *_str, unsigned _len); //fill a char array with null bytes
int get_strings(int *strc_, char **strv_, const char *_fname); //gets strings from config file
int read_cfg_file(int *strc_, char **strv_, const char *_fname);
int main_menu(char *_cfg_fname);
int perf_opt(struct cfg_entry *_f_to_call);//perform cfg_entry assigned function

int main(int argc, char **argv)
{
    init_nc();

    char *cfg_fname = NULL;
    parse_args(argc, argv, &cfg_fname);

    main_menu(cfg_fname);

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

    return 0;
}

int parse_args(int _argc, char **_argv, char **_cfg_fname)
{
    if(_argc > 1 && (strcmp(_argv[1], "-c") == 0)) {
        if(_argc < 3) {
            mvprintw(0,0, "config flag needs file-name parameter!");
            getch();
            return 1;
        }
        else { *_cfg_fname = _argv[2]; }
    }

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

    unsigned ff_i = 0; //iterator for fast forwarding

    move(0,0);
    if(file) {
        int ch = 0;
        int cmd = 0;

        clear();

        halfdelay(1); //getch() will wait a bit and then continue
        //quickly printing some characters that usually appear in bulk
        while((ch = getc(file)) != EOF) {
            addch(ch);

            if(ff_i > 0) { --ff_i; continue; }

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
            if(cmd == 'j') { ff_i = 100; } //fast forward 100ch
            else if (cmd == 'g') { ff_i = 1000; } //ff 1000ch
            else if(cmd != ERR) { break; }
        }
        cbreak(); //getch() will wait for input indefinitely (interruptable)
    }
    else {
        clear();
        mvprintw(0, 0, "ERROR(%d - %s): could not read %s\n",
                err, strerror(errno), _fpath);
        getch();
        clear();
        return -1;
    }
    

    mvprintw(getmaxy(stdscr) - 1, 0, "press any key to continue");
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

int main_menu(char *_cfg_fname)
{
    int cmd = 0;

    char daily_fpath[256];
    char todo_fpath[256];

    int default_sel = 0;

    const char menu_header[] = "********** MAIN MENU **********";
    const char menu_footer[] = "*******************************";
    const int menu_w = strlen(menu_header);

    int opt_count = 0;
    //TODO remove the max_strc limitation, and make the menu scrollable
    unsigned max_strc = 20;
    char *strv[max_strc];
    read_cfg_file(&opt_count, strv, _cfg_fname);
    //TODO if no config found, read defaults.cfg (implement)

    char **menu_lines = calloc(opt_count, sizeof menu_lines[0]);
    int last_sel = opt_count - 1;

    char *cfg_delim = ";";
    struct cfg_entry **cfg = calloc(opt_count, sizeof cfg[0]);
    for(unsigned i = 0; i < opt_count; ++i) {
        //allocating memory
        cfg[i] = malloc(sizeof cfg[i]);
        if(str_to_cfg(strv[i], cfg_delim, cfg[i]) < 0) { return -1; }
        menu_lines[i] = malloc(strlen(cfg[i]->menu_txt) + 1); //+1 for null-term

        strcpy(menu_lines[i], cfg[i]->menu_txt);
    }

    char date_str[64];
    const int menu_min_y = 0;
    const int opts_start_pos = 2;
    int sel = default_sel;
    while(cmd != 'q') {
        time_t t = time(NULL);
        struct tm *tm_st = localtime(&t);

        strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S %A", tm_st);

        print_mid(menu_min_y, date_str);
        int menu_min_x = print_mid(menu_min_y + 1, menu_header);

        //print menu options
        for(unsigned i = 0; i < opt_count; ++i) {
            if(i == sel) { //paint selected menu option differently
                attron(A_REVERSE);
            }
            mvprintw(i + opts_start_pos + menu_min_y, menu_min_x + 2, menu_lines[i]);
            attroff(A_REVERSE);

            //side decorations
            mvprintw(i + opts_start_pos + menu_min_y, menu_min_x,
                    "*");
            mvprintw(i + opts_start_pos + menu_min_y, menu_min_x + menu_w - 1,
                    "*");
        }
        //print arrow left to selected option
        mvprintw(sel + opts_start_pos + menu_min_y, menu_min_x,
                ">");

        mvprintw(opt_count + opts_start_pos + menu_min_y, menu_min_x,
                menu_footer);

        //display version number on the bottom
        print_mid(getmaxy(stdscr) - 1, VERSION);

        halfdelay(10); //wait a sec (10/10 of a second) for input
        cmd = getch();
        cbreak(); //turn of input wait limit

        if(cmd == ERR) { continue; }

        if(cmd == 'k' && sel > 0) { --sel; }
        else if(cmd == 'j' && sel < opt_count - 1) { ++sel; }
        else if(cmd == 'l') {
            int rc = perf_opt(cfg[sel]);
            if(rc == 1) { break; }
            continue;
        }
    }

    for(unsigned i = 0; i < max_strc && i < opt_count; ++i) {
        free(strv[i]);
        free(menu_lines[i]);
        free_cfg_entry(cfg[i]);
    }
    free(menu_lines);
    free(cfg);

    return 0;
}

int zeroize_str(char *_str, unsigned _len)
{
    for(unsigned i = 0; i < _len; ++i) {
        _str[i] = '\0';
    }

    return 0;
}

int read_cfg_file(int *strc_, char **strv_, const char *_fname)
{
    char dfname[] = "config.cfg"; //default config filename
    const char *fname = _fname;
    if(fname == NULL) { fname = dfname; }

    char buf[MAX_CFG_LN_LEN];
    zeroize_str(buf, sizeof(buf));
    FILE *file;

    file = fopen(fname, "r");
    if(!file) {
        clear();
        mvprintw(0, 0, "error opening main.cfg (%s)", strerror(errno));
        getch();
        return -1;
    }

    int strc = 0;
    while(1) {
        fgets(buf, sizeof(buf) - 1, file);
        if(feof(file)) { break; }

        //null the new-line char if existant
        if(buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }

        strv_[strc] = malloc(strlen(buf) + 1);
        strcpy(strv_[strc], buf);
        ++strc;
    }
    fclose(file);

    *strc_ = strc;

    return 0;
}

int perf_opt(struct cfg_entry *_opt)
{
    if(strcmp(_opt->f_to_call, "slow_print") == 0) {
        slow_print_file(_opt->path); }
    else if(strcmp(_opt->f_to_call, "edit") == 0) { edit_file(_opt->path); }
    else if(strcmp(_opt->f_to_call, "quit") == 0) { return 1; }

    return 0;
}
