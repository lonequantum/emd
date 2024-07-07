#ifndef _MAIN_H
#define _MAIN_H


#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>


#include <libevdev.h>
typedef struct libevdev LIBEVDEV;


#define NO_DAEMON_ENVVAR_NAME  "NODAEMON"
#define SHELL_COMMAND_MAX_LEN  255

typedef struct config {
	int type, code, value;
	char command[SHELL_COMMAND_MAX_LEN];
	struct config *next;
} CONFIG;
#define CONFIG_ITEM_MAX_LEN    SHELL_COMMAND_MAX_LEN


// main.c
void      clean(void);
void      daemonize(void);
void      detect_instances(const char *const);
void      register_signals(void);
void      report(const int, const char *const, ...);
void      show_usage(FILE *const);
void      terminate(const int);

// load.c
CONFIG   *get_config(const char *const);
LIBEVDEV *get_dev(const char *const);

// exec.c
void      process_event();


#endif
