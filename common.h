#ifndef CAPYBARA_COMMON_H
#define CAPYBARA_COMMON_H
#include "modules.h"
enum be_type {
    CALL_FN,
    APPEND_STR
};

extern DPA *symbols;
extern DPA *bar_entries;
extern DPA *poll_fds;
extern char *fontstr;
extern char *dimstr;

struct symbol_entry {
    cbrm_disp_fn dfn;
    char *name;
};
struct bar_entry {
    enum be_type type;
    void *ptr;
};


extern void load_ext(char *name);
extern void parse_cfg(char *filename);
#endif
