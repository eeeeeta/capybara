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
#include <ctype.h>
#include <string.h>
#include <dlfcn.h>
#include "varlibs/vbuf.h"
#include "common.h"

#define RARROW ""
#define LARROW ""
DPA *symbols;
DPA *bar_entries;


int main(int argc, char **argv) {
    printf("[+] Capybara version 0.1.0\n");
    printf("[*] an eta thing <http://theta.eu.org>\n");
    symbols = DPA_init();
    bar_entries = DPA_init();
    printf("[+] Reading and parsing configuration...\n");
    parse_cfg("bogus.cfg");
    printf("[+] Successfully parsed %d instructions.\n", bar_entries->used);
}
