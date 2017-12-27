#ifndef CONFIG_H
#define CONFIG_H

//c std lib
#include <stdio.h>
#include <assert.h>
#include <string.h>

//linux
#include <ncurses.h>

//TODO PRI_1 make a function to free memory allocated to cfg_entry members
struct cfg_entry {
    char *menu_txt; //menu tex for the entry
    char *f_to_call; //TODO this should probably be a pointer to a callback
    char *path; //path to target file
};

//load config entry from string
int str_to_cfg(char *_str, const char *_delim, struct cfg_entry *cfg_);
int print_cfg_entry(struct cfg_entry *_cfg); //for debugging

#endif //ifndef CONFIG_H
