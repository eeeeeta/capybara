/*
 * native version of capybara
 * NEVER TO BE FINISHED EVER
 *
 * -eta
 */
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include "varlibs/vbuf.h"
#include "modules.h"
#define RARROW ""
#define LARROW ""
#define GRAY "%{F#808080}"
#define WHITE "%{F#FFFFFF}"
#define ESYMBOL(arg) ((struct symbol_entry *) EITEM(arg))
enum parser_fsm {
    READ_TYPE,
    READ_DONE,
    READ_VALUE,
    COMMENT
};

DPA *symbols;
DPA *bar_entries;
struct symbol_entry {
    cbrm_disp_fn dfn;
    char *name;
};
struct bar_entry {
    bool call;
    void *ptr;
};
bool mod_register(char *sym, cbrm_disp_fn dfn) {
    struct symbol_entry *sa = malloc(sizeof(struct symbol_entry));
    if (sa == NULL) err(EXIT_FAILURE, "mod_register(): malloc failed");
    sa->dfn = dfn;
    sa->name = sym;
    DPA_store(symbols, sa);
    printf("[+] Registered symbol %s.\n", sym);
}
void load_ext(char *name) {
    void *handle = NULL;
    int *ver = NULL, mver = 0;
    char *syms = NULL, *error = NULL, *extname = NULL;
    handle = dlopen(name, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "[-] error loading extension %s: %s\n", name, dlerror());
        return;
    }
    dlerror();
    /*
     * quite a lot of the code below is pilfered from atheme/charybdis' loading functions
     */
    ver = (int *) dlsym(handle, "_cbrm_header");
    if ((error = dlerror()) != NULL) {
        fprintf(stderr, "[-] error loading extension %s: extension has no _cbrm_header\n", name);
        fprintf(stderr, "[-] error loading extension %s: %s\n", name, error);
        dlclose(handle);
        return;
    }
    if (CBRM_MAGIC(*ver) != CBRM_MAGIC_HDR) {
        fprintf(stderr, "[-] extension %s does not have a correct magic header\n", name);
        dlclose(handle);
        return;
    }
    switch (CBRM_VERSION(*ver)) {
        case 1:
            {
                struct cbrm_header_v1 *hdr = (struct cbrm_header_v1 *) (void *) ver;
                if (!hdr->regfn) {
                    fprintf(stderr, "[*] extension %s has no register function. what exactly is it meant to do?! :P\n", name);
                }
                if (hdr->regfn && !hdr->regfn(&mod_register)) {
                    fprintf(stderr, "[-] extension %s indicated failure on load :(\n", name);
                    dlclose(handle);
                    return;
                }
                mver = hdr->mver;
                extname = hdr->name;
            }
            break;
        default:
            fprintf(stderr, "[-] extension %s has unknown API version %d\n", name, CBRM_VERSION(*ver));
            dlclose(handle);
            return;

    }
    printf("[+] Loaded extension %s [version %d, API version %d] from file %s.\n", extname, mver, CBRM_VERSION(*ver), name);
    return;
}
void parse_cfg(char *filename) {
    FILE *fd = fopen(filename, "r");
    if (!fd) err(EXIT_FAILURE, "could not open configuration file %s", filename);
    char c = '\0';
    enum parser_fsm state = READ_TYPE;
    char type = 'Z';
    struct varstr *value = varstr_init();
    while (c = fgetc(fd)) {
        if (c == '\n' || c == EOF) {
            if (state == READ_VALUE) {
                switch (type) {
                    case 'L':
                        {
                            char *filename = varstr_pack(value);
                            load_ext(filename);
                        }
                        break;
                    case 'C':
                        {
                            struct bar_entry *be = malloc(sizeof(struct bar_entry));
                            if (be == NULL) err(EXIT_FAILURE, "parse_cfg(): malloc failed");
                            char *fname = varstr_pack(value);
                            be->call = true;
                            be->ptr = NULL;
                            DPA_ENUMERATE(symbols) {
                                if (strcmp(ESYMBOL(symbols)->name, fname) == 0) {
                                    be->ptr = ESYMBOL(symbols)->dfn;
                                    break;
                                }
                            }
                            if (be->ptr == NULL) {
                                warnx("C-line attempted to use unknown symbol %s", fname);
                            }
                            else {
                                DPA_store(bar_entries, be);
                            }
                        }
                        break;
                    default:
                        warnx("found unknown configuration line type: %c\n", type);
                        varstr_free(value);
                }
                value = varstr_init();
            }
            else if (state != COMMENT && state != READ_TYPE) {
                errx(EXIT_FAILURE, "failed to parse config: unexpected newline/EOF");
            }
            state = READ_TYPE;
            if (c == EOF) break;
            continue;
        }
        switch (state) {
            case READ_TYPE:
                if (c == '#') state = COMMENT;
                else {
                    type = c;
                    state = READ_DONE;
                }
                break;
            case READ_DONE:
                if (c != ' ') {
                    errx(EXIT_FAILURE, "failed to parse config: no space after line type\n");
                }
                else {
                    state = READ_VALUE;
                }
                break;
            case READ_VALUE:
                varstr_pushc(value, c);
                break;
            case COMMENT:
                break;
            default:
                errx(EXIT_FAILURE, "this shouldn't happen :P");
        }
    }
}
int main(int argc, char **argv) {
    printf("[+] Capybara version 0.1.0\n");
    printf("[*] an eta thing <http://theta.eu.org>\n");
    symbols = DPA_init();
    bar_entries = DPA_init();
    printf("[+] Reading and parsing configuration...\n");
    parse_cfg("bogus.cfg");
}
