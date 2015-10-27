#ifndef CAPYBARA_MODULES_H
#define CAPYBARA_MODULES_H
#include <stdbool.h>

/*
 * the below definitions are stolen from charybdis IRCD's module loading stuff
 * what, you thought I could come up with this myself? ;p
 */

#define CBRM_MAGIC_HDR 0xC0FF0000
#define CBRM_V1 (CBRM_MAGIC_HDR | 0x1)
#define CBRM_MAGIC(x)	((x) & 0xffff0000)
#define CBRM_VERSION(x)	((x) & 0x0000ffff)

typedef char *(*cbrm_disp_fn) (void);
typedef bool (*cbrm_reg_fn) (char *name, cbrm_disp_fn dfn);
typedef bool (*cbrm_register_fn) (cbrm_reg_fn regfn);

struct cbrm_header_v1 {
    int apiver;
    char *name;
    unsigned int mver;
    cbrm_register_fn regfn;
};

#define CBR_DECLARE_MODULE_V1(name, version, regfn) struct cbrm_header_v1 _cbrm_header = {CBRM_V1, name, version, regfn};

#endif
