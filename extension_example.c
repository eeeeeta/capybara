/*
 * extension for capybara example
 */

#include <stdio.h>
#include <unistd.h>
#include "modules.h"
char *example_function(void) {
    return "hello, world";
};
bool module_register(cbrm_reg_fn regfn, cbrm_reg_fd_fn regfdfn) {
    regfn("example_function", &example_function);
    return true;
}
CBR_DECLARE_MODULE_V1("helloworld_ext", 1, &module_register);
