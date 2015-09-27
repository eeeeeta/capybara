/*
 * crappy C lemonbar manager
 * by eeeeeta
 *
 * v0.0.2: now with STATIC DECLARATIONS
 * (because you hate malloc) - and hostname display!
 */
#define GRAY "%{F#808080}"
#define WHITE "%{F#FFFFFF}"
#define MUSIC "♫"
#define RARROW ""
#define LARROW ""
#include <stdio.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

FILE *barfp, *xtfp, *bspfp;
static char curtitle[41] = {0};
char *callocr(int sz) {
    char *ret;
    if ((ret = calloc(sz, 1)) == NULL) {
        perror("calloc failed");
        pclose(barfp);
        exit(EXIT_FAILURE);
    }
    return ret;
}
static char *exec(char *cmd) {
    FILE *fp;
    char *state = callocr(175);
    if ((fp = popen(cmd, "r")) == NULL) {
        printf("popen failed\n");
        if (errno != 0) perror("popen() failed");
        strncpy(state, "popen() failed!", 100);
        return state;
    }
    if ((errno = 0) == 0 && fgets(state, 175, fp) == NULL && errno != 0) {
        perror("fgets failed");
        strncpy(state, "fgets() failed!", 100);
        return state;
    }
    if ((pclose(fp)) == -1) {
        printf("warn: pclose failed\n");
        perror("pclose");
    }
    int len = strlen(state);
    if (len != 0 && state[len-1] == '\n') state[len-1] = '\0';
    return state;
}
static char *get_load() {
    static double loadavg[3] = {0};
    static char ret[50] = {0};
    if ((getloadavg(loadavg, 3)) == -1) {
        strncpy(ret, "err", 14);
        return ret;
    }
    snprintf(ret, 50, "%s %sload%s %1.2f", LARROW, GRAY, WHITE, loadavg[0]);
    return ret;
}
#ifndef NO_PULSEAUDIO
static char *get_vol() {
    char *vol = exec("pulseaudio-ctl full-status");
    if (strcmp(vol, "failed") == 0) return vol;
    static char ret[100] = {0};
    char *rvol = strtok(vol, " ");
    char *vstr = "vol";
    if (strcmp(strtok(NULL, " "), "yes") == 0) vstr = "muted";
    snprintf(ret, 100, "%s %s%s %s%s%%", LARROW, GRAY, vstr, WHITE, rvol);
    free(vol);
    return ret;
}
#else
static char *get_vol() {
    return "";
}
#endif
static char *get_time() {
    static char ret[50] = {0};
    static char stime[45] = {0};
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strcpy(ret, LARROW);
    strcat(ret, " ");
    strftime(stime, 45, "%%{F#808080}time %%{F#FFFFFF}%H:%M", timeinfo);
    strcat(ret, stime);
    return ret;
}
#ifndef NO_MPD
static char *get_mpd() {
    static char ret[175] = {0};
    static char *res = NULL;
    res = exec("mpc current -f \"[%{F##808080}title%{F##FFFFFF} %title% %{F##808080}artist %{F##FFFFFF}%artist%[ %{F##808080}album %{F##FFFFFF}%album%]]\"");
    strcpy(ret, res);
    free(res);
    return ret;
};
#else
static char *get_mpd() {
    return "";
}
#endif
static char *get_hname() {
    static char ret[75] = {0};
    static char hname[15] = {0};
    if (gethostname(hname, 15) == -1) {
        strcpy(hname, "unknown");
    }
    strcpy(ret, GRAY);
    strcat(ret, "on ");
    strcat(ret, WHITE);
    strcat(ret, hname);
    strcat(ret, " ");
    strcat(ret, RARROW);
    return ret;
}
static char *get_win(bool cached) {
    static char bspstr[201] = {0};
    static char win[26] = {0};
    static char uwin[26] = {0};
    if (!cached) {
        while (fgets(bspstr, 200, bspfp) != NULL);
        if (errno != EWOULDBLOCK) {
            perror("fgets failed");
        }
        strtok(bspstr, "\n");
        /*
         * rat's nest of inelegant flow below!
         */
        if (bspstr[0] == 'W' && bspstr[1] == 'M') {
            int state = 0, i = 2, stri = 0, ustri = 0;
            memset(win, 0, 26);
            memset(uwin, 0, 26);
            for (char c = bspstr[i]; i < strlen(bspstr); i++) {
                c = bspstr[i];
                if (state == 1) {
                    if (c == 'F' || c == 'O' || c == 'U') state = 2;
                    else if (c == 'u') state == 3;
                    else state = 0;
                }
                else if (c == ':') state = 1;
                else if (state == 2) {
                    if (c == ':') {
                        win[strlen(win)] = '\0';
                        state = 0;
                    }
                    else {
                        if (stri > 24) {
                            win[strlen(win)] = '\0';
                            state = 0;
                        }
                        else {
                            win[stri++] = c;
                        }
                    }
                }
                else if (state == 3) {
                    if (c == ':') {
                        uwin[strlen(uwin)] = '\0';
                        state = 0;
                    }
                    else {
                        if (stri > 24) {
                            win[strlen(uwin)] = '\0';
                            state = 0;
                        }
                        else {
                            uwin[ustri++] = c;
                        }
                    }
                }
            }
        }
    }
    static char ret[150] = {0};
    strcpy(ret, GRAY);
    strcat(ret, "desktop ");
    strcat(ret, WHITE);
    strcat(ret, win);
    if (strlen(uwin) > 0) {
        strcat(ret, " (URG: ");
        strcat(ret, uwin);
        strcat(ret, ")");
    }
    return ret;
}
static char *get_uname() {
    static char *uname;
    static char ret[50] = {0};
    if ((uname = getlogin()) == NULL && (uname = getenv("USER")) == NULL) uname = "no idea";
    strcpy(ret, LARROW);
    strcat(ret, " ");
    strcat(ret, GRAY);
    strcat(ret, "user ");
    strcat(ret, WHITE);
    strcat(ret, uname);
    return ret;
}
static char *get_xtitle(bool cached) {
    static char cxtitle[51] = {0};
    static char ret[75] = {0};
    if (!cached) {
        while (fgets(cxtitle, 50, xtfp) != NULL);
        if (errno != EWOULDBLOCK) {
            perror("fgets failed");
        }
        strtok(cxtitle, "\n");
        if (cxtitle[0] == '\n') strcpy(cxtitle, "none");
    }
    strcpy(ret, GRAY);
    strcat(ret, "win ");
    strcat(ret, WHITE);
    strcat(ret, cxtitle);
    strcat(ret, " ");
    strcat(ret, RARROW);
    return ret;
}
void bar_upd(int sig) {
    static char barstr[750];
    snprintf(barstr, 750, "%%{l}%s %s %s%%{c}%s%%{r}%s %s %s %s\n",
            get_win((sig == -2 ? false : true)),
            get_hname(),
            get_xtitle((sig == -1 ? false : true)),
            get_mpd(),
            get_vol(),
            get_load(),
            get_uname(),
            get_time());
    fputs(barstr, barfp);
    fflush(barfp);
}
int main(int argc, char *argv[]) {
    printf("Welcome to Capybara, eta's lemonbar manager\n");
#ifdef NO_MPD
    printf("Compiled with MPD (through mpc) disabled.\n");
#endif
#ifdef NO_PULSEAUDIO
    printf("Compiled with pulseaudio-ctl disabled.\n");
#endif
    printf("[+] Setting up...\n");
    printf("spawning lemonbar...");
    if ((barfp = popen("lemonbar -g 1890x20+15+10 -f \"Inconsolata for Powerline\"", "w")) == NULL) {
        printf("failed (is it installed and in your PATH?)\n");
        if (errno != 0) perror("popen() failed");
        return EXIT_FAILURE;
    }
    printf("done\n");
    printf("spawning xtitle...");
    if ((xtfp = popen("xtitle -s -t 40", "r")) == NULL) {
        printf("failed (is it installed and in your PATH?)\n");
        if (errno != 0) perror("popen() failed");
        return EXIT_FAILURE;
    }
    printf("done\n");
    printf("spawning bspc control --subscribe...");
    if ((bspfp = popen("bspc control --subscribe", "r")) == NULL) {
        printf("failed (is it installed and in your PATH? can bspc connect to bspwm's socket?)\n");
        if (errno != 0) perror("popen() failed");
        return EXIT_FAILURE;
    }
    printf("done\n");
    void bar_upd(int sig);
    struct sigaction sa;
    sa.sa_handler = bar_upd;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }
    struct pollfd ufds[2];
    ufds[0].fd = fileno(xtfp);
    ufds[0].events = POLLIN;
    ufds[1].fd = fileno(bspfp);
    ufds[1].events = POLLIN;
    int flags;
    flags = fcntl(fileno(xtfp), F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fileno(xtfp), F_SETFL, flags);
    flags = fcntl(fileno(bspfp), F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fileno(bspfp), F_SETFL, flags);
    printf("[+] Bar setup completed!\n");
    printf("[+] The bar will autoupdate intelligently on its own. Send it a SIGUSR1 to manually refresh.\n");
    int rv = -1;
    while (1) {
        rv = poll(ufds, 1, 5000);
        if (rv == -1) {
            if (errno == EINTR) {
                bar_upd(SIGUSR1);
                continue;
            }
            perror("poll");
        }
        else if (rv == 0) bar_upd(SIGUSR1);
        else {
            if (ufds[0].revents & POLLIN) bar_upd(-1);
            if (ufds[1].revents & POLLIN) bar_upd(-2);
        }
    }
}
