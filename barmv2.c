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
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <dlfcn.h>
#include "varlibs/vbuf.h"
#include "common.h"

#define RARROW ""
#define LARROW ""
#define EBARENTRY ((struct bar_entry *) EITEM(bar_entries))
#define EPOLLFD ((struct pollfd *) EITEM(poll_fds))

DPA *symbols;
DPA *bar_entries;
DPA *poll_fds;

char *fontstr = NULL;
char *dimstr = NULL;

FILE *lemonbarfd;

char *generate_barstr(void) {
    struct varstr *barstr = varstr_init();
    DPA_ENUMERATE(bar_entries) {
        switch (EBARENTRY->type) {
            case APPEND_STR:
                varstr_cat(barstr, (char *) EBARENTRY->ptr);
                break;
            case CALL_FN:
                varstr_cat(barstr, ((cbrm_disp_fn) EBARENTRY->ptr)());
                break;
        }
    }
    varstr_cat(barstr, "\n");
    return varstr_pack(barstr);
}
void bar_update(int signal) {
    char *bstr = generate_barstr();
    fputs(bstr, lemonbarfd);
    fflush(lemonbarfd);
}
int main(int argc, char **argv) {
    printf("[+] Capybara version 0.1.0\n");
    printf("[*] an eta thing <http://theta.eu.org>\n");
    symbols = DPA_init();
    bar_entries = DPA_init();
    poll_fds = DPA_init();
    printf("[+] Reading and parsing configuration...\n");
    parse_cfg("bogus.cfg");
    printf("[+] Successfully parsed %d instructions.\n", bar_entries->used);
    printf("[+] Setting up lemonbar command line...\n");
    struct varstr *cmdline = varstr_init();
    varstr_cat(cmdline, "lemonbar");
    if (fontstr) {
        char *tok = strtok(fontstr, ",");
        while (tok) {
            varstr_cat(cmdline, " -f \"");
            varstr_cat(cmdline, tok);
            varstr_cat(cmdline, "\"");
            tok = strtok(NULL, ",");
        }
    }
    if (dimstr) {
        varstr_cat(cmdline, " -g ");
        varstr_cat(cmdline, dimstr);
    }
    printf("[+] Spawning lemonbar...\n");
    errno = 0;
    if ((lemonbarfd = popen(varstr_pack(cmdline), "w")) == NULL) {
        printf("[-] Failed to spawn lemonbar (is it in your PATH?).\n");
        if (errno != 0) err(EXIT_FAILURE, "failed to spawn lemonbar");
        else errx(EXIT_FAILURE, "failed to spawn lemonbar");
    }
    printf("[+] Setting up poll()...\n");
    int npfds = poll_fds->used + 1;
    struct pollfd pfds[npfds];
    pfds[0].fd = fileno(lemonbarfd);
    pfds[0].events = POLLIN;
    DPA_ENUMERATE(poll_fds) {
        pfds[DPA_N + 1].fd = EPOLLFD->fd;
        pfds[DPA_N + 1].events = EPOLLFD->events;
    }
    struct sigaction sa;
    sa.sa_handler = bar_update;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        err(EXIT_FAILURE, "sigaction failed");
    }
    printf("[+] Complete!\n");
    bar_update(SIGUSR1);
    printf("[*] Capybara will automatically update if your extensions support it.\n");
    printf("[*] You may send it a SIGUSR1 to force an update. Updates will also happen every 10 seconds.\n");
    int rv = -1;
    while (1) {
        rv = poll(pfds, npfds, 10000);
        if (rv == -1) {
            if (errno == EINTR) {
                bar_update(SIGUSR1);
                continue;
            }
            perror("poll");
        }
        bar_update(SIGUSR1);
    }
}
