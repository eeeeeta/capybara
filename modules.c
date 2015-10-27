/*
 * DSO module loading
 * (largely pilfered from charybdis)
 */
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <dlfcn.h>
#include "modules.h"
#include "varlibs/vbuf.h"
#include "common.h"
extern bool mod_register(char *sym, cbrm_disp_fn dfn) {
    struct symbol_entry *sa = malloc(sizeof(struct symbol_entry));
    if (sa == NULL) err(EXIT_FAILURE, "mod_register(): malloc failed");
    sa->dfn = dfn;
    sa->name = sym;
    DPA_store(symbols, sa);
    printf("[+] Registered symbol %s.\n", sym);
}
extern void load_ext(char *name) {
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
