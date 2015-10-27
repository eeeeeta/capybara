/*
 * extension for capybara example
 */

#include <stdio.h>
#include "modules.h"
char *example_function(void) {
    return "woohoo!";
};
bool module_register(cbrm_reg_fn regfn) {
    printf("example extension: registering\n");
    regfn("example_function", &example_function);
    printf("yay, done\n");
    return true;
}
CBR_DECLARE_MODULE_V1("example_module", 1, &module_register);
