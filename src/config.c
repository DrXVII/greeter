#include "config.h" //the header for this implementation file

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

void free_cfg_entry(struct cfg_entry *_entry)
{
    //TODO do not point outside of struct for below members, especially since there are plans to make f_to_call a pointer to function
    /* menu_txt, f_to_call, path currently point to offsets of *_str 
     * and are freed when _str is freed*/
    //free(_entry->menu_txt);
    //free(_entry->f_to_call);
    //free(_entry->path);
    free(_entry);
}

int print_cfg_entry(struct cfg_entry *_cfg) //for debugging
{
    printf("menu_txt = %s\n", _cfg->menu_txt);
    printf("f_to_call = %s\n", _cfg->f_to_call);
    printf("path = %s\n", _cfg->path);
}
