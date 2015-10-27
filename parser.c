/*
 * capybara v2's config file parser
 * uses a tiny fixed state machine
 */
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include "varlibs/vbuf.h"
#include "common.h"
#define ESYMBOL(arg) ((struct symbol_entry *) EITEM(arg))
enum parser_fsm {
    READ_TYPE,
    READ_DONE,
    READ_VALUE,
    COMMENT
};
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
                switch (toupper(type)) {
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
                            be->type = CALL_FN;
                            be->ptr = NULL;
                            DPA_ENUMERATE(symbols) {
                                if (strcmp(ESYMBOL(symbols)->name, fname) == 0) {
                                    be->ptr = ESYMBOL(symbols)->dfn;
                                    break;
                                }
                            }
                            if (be->ptr == NULL) {
                                warnx("C-line: attempted to use unknown symbol %s", fname);
                                free(be);
                            }
                            else {
                                DPA_store(bar_entries, be);
                            }
                        }
                        break;
                    case 'M':
                        {
                            struct bar_entry *be = malloc(sizeof(struct bar_entry));
                            if (be == NULL) err(EXIT_FAILURE, "parse_cfg(): malloc failed");
                            be->type = APPEND_STR;
                            be->ptr = NULL;
                            char *arg = varstr_pack(value);
                            switch (tolower(arg[0])) {
                                case 'l':
                                    be->ptr = "%{l}";
                                    break;
                                case 'c':
                                case 'm':
                                    be->ptr = "%{c}";
                                    break;
                                case 'r':
                                    be->ptr = "%{c}";
                                    break;
                                default:
                                    warnx("M-line: unknown argument %s (expected l, m or r)\n", arg);
                                    break;
                            }
                            if (be->ptr != NULL) DPA_store(bar_entries, be);
                            else free(be);
                        }
                        break;
                    case 'O':
                        {
                            struct bar_entry *be = malloc(sizeof(struct bar_entry));
                            if (be == NULL) err(EXIT_FAILURE, "parse_cfg(): malloc failed");
                            be->type = APPEND_STR;
                            char *arg = varstr_pack(value);
                            struct varstr *newstr = varstr_init();
                            char *what = strtok(arg, " ");
                            char *how = strtok(NULL, " ");
                            if (what == NULL || how == NULL) {
                                warnx("O-line: too little arguments (expected 2)\n");
                                goto oline_failure; /* ZOMG Batman, a goto statement!!!1111!! */
                            }
                            switch (tolower(what[0])) {
                                case 'f':
                                    varstr_cat(newstr, "%{F");
                                    break;
                                case 'b':
                                    varstr_cat(newstr, "%{B");
                                    break;
                                case 'u':
                                    varstr_cat(newstr, "%{U");
                                    break;
                                default:
                                    warnx("O-line: invalid target (expected f, b or u)\n");
                                    goto oline_failure;
                                    break;
                            }
                            varstr_cat(newstr, how);
                            varstr_cat(newstr, "}");
                            be->ptr = varstr_pack(newstr);
                            DPA_store(bar_entries, be);
                            if ((1 + 1) != 2) {
oline_failure:                  free(be);
                                varstr_free(newstr);
                            }
                        }
                        break;
                    case 'F':
                        fontstr = varstr_pack(value);
                        printf("[+] Changed font configuration to %s\n", fontstr);
                        break;
                    case 'I':
                        dimstr = varstr_pack(value);
                        printf("[+] Changed dimension configuration to %s\n", dimstr);
                        break;
                    case 'Z':
                        errx(EXIT_FAILURE, "this should never happen (unless I screwed up somewhere!)");
                        break;
                    default:
                        warnx("found unknown configuration line type: %c", type);
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
                if (c == ' ') {
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
