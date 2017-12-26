//c std lib
#include <stdio.h> //printf(), scanf(), fopen(), etc
#include <stdlib.h>
#include <string.h> //strlen(), strerror()
#include <time.h> //time(), localtime()
#include <errno.h>
#include <assert.h>

//linux
#include <ncurses.h>

#define MAX_CMD_LEN 1024
#define MAX_CFG_LN_LEN 1024
#define MAX_MEN_LNS 20
#define VERSION "v1.0.2"

int init_nc(void); //initialise ncurses
int view_file(const char *_fpath); //function for viewing files
int edit_file(const char *_fpath); //function for editing files
int slow_print_file(const char *_fpath); //prints file slowly char by char
int print_mid(int _y, const char* _s); //print in middle of screen (on x axis)
int zeroize_str(char *_str, unsigned _len); //fill a char array with null bytes
int get_strings(int *strc_, char **strv_, const char *_fname); //gets strings from config file
int read_cfg_file(int *strc_, char **strv_, const char *_fname);
int main_menu(void);

//TODO PRI_2 implement dynamic loading allocation of menu options, instead of the quick and dirty solution that "just works" and is hardcoded

//cfg_entry and associated functions should be moved to appropriate header TODO-
//TODO PRI_1 make a function to free memory allocated to cfg_entry members
struct cfg_entry {
    char *menu_txt; //menu tex for the entry
    char *f_to_call; //TODO this should probably be a pointer to a callback
    char *path; //path to target file
};

int print_cfg_entry(struct cfg_entry *_cfg) { //for debugging
    printf("menu_txt = %s\n", _cfg->menu_txt);
    printf("f_to_call = %s\n", _cfg->f_to_call);
    printf("path = %s\n", _cfg->path);
}

int str_to_cfg(char *_str, const char *_delim, struct cfg_entry *cfg_)
{
    assert(_str != NULL);
    assert(_delim != NULL);
    assert(cfg_ != NULL);

    cfg_->menu_txt = strsep(&_str, _delim);
    cfg_->f_to_call = strsep(&_str, _delim);
    cfg_->path = strsep(&_str, _delim);

    if(cfg_->menu_txt == NULL
    || cfg_->f_to_call == NULL
    || cfg_->path == NULL) {
        mvprintw(0, 0, "error while parsing config!!");
        getch();
        return -1;
    }
}
//------------------------------------------------------------------------------

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
        //quickly printing some characters that usually appear in bulk
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

int main_menu(void)
{
    int cmd = 0;

    const char cfg_fname[] = "config.cfg";

    char daily_fpath[256];
    char todo_fpath[256];

    enum Opts {daily = 0, todo, quit};
    int default_sel = daily;
    int last_sel = quit;

    int opt_count = last_sel + 1;
    char *entries[opt_count]; 

    const char menu_header[] = "********** MAIN MENU **********";
    const char menu_footer[] = "*******************************";
    const int menu_w = strlen(menu_header);

    int strc = 0;
    int *p_strc = &strc;
    unsigned max_strc = 20;
    char *strv[max_strc];
    read_cfg_file(p_strc, strv, cfg_fname);
    //TODO if no config found, read defaults.cfg

    char *cfg_delim = ";";
    struct cfg_entry *cfg[2];
    cfg[0] = malloc(sizeof cfg[0]);
    cfg[1] = malloc(sizeof cfg[1]);
    if(strc > 0) {
        if(str_to_cfg(strv[0], cfg_delim, cfg[0]) < 0) {
            return -1;
        }
    }
    if(strc > 1) {
        if(str_to_cfg(strv[1], cfg_delim, cfg[1]) < 0) {
            return -1;
        }
    }

    entries[daily] = cfg[0]->menu_txt;
    entries[todo] = cfg[1]->menu_txt;
    entries[quit] = "quit to terminal";

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
                attron(COLOR_PAIR(1));
            }
            mvprintw(i + opts_start_pos + menu_min_y, menu_min_x + 2, entries[i]);
            attroff(COLOR_PAIR(1)); //returning to default color pair

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

        if(cmd != 'l') { continue; }

        switch(sel) {
            case daily: slow_print_file(cfg[0]->path); break;
            case todo: edit_file(cfg[1]->path); break;
            case quit: cmd = 'q'; break;
            default: sel == default_sel; break;
        }
    }

    for(unsigned i = 0; i < max_strc && i < strc; ++i) {
        free(strv[i]);
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

int read_cfg_file(int *strc_, char **strv_, const char *_fname)
{
    char buf[MAX_CFG_LN_LEN];
    zeroize_str(buf, sizeof(buf));
    FILE *file;

    file = fopen("config.cfg", "r");
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
