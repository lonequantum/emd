#include "main.h"


struct input_event  Event;
LIBEVDEV           *Dev;
CONFIG             *Config;
_Bool               Is_daemon;
_Bool               Critical_error;


// Send a message to syslog (if we run as a daemon) or to standard streams.
void report(const int level, const char *const format, ...)
{
	va_list args;
	va_start(args, format);
	if (Is_daemon)
		vsyslog(level, format, args);
	else
		vfprintf(
		level & (LOG_DEBUG | LOG_INFO | LOG_NOTICE) ? stdout : stderr,
		format, args
		);
	va_end(args);
}


// Free the allocated resources.
void clean(void)
{
	if (Dev)
	{
		close(libevdev_get_fd(Dev));
		libevdev_free(Dev);
	}
	while (Config)
	{
		CONFIG *c = Config;
		Config = c->next;
		free(c);
	}
}


// Quit the program properly.
void terminate(const int sig_number)
{
	report(LOG_NOTICE, "received signal %d, exiting\n", sig_number);
	clean();
	switch (sig_number)
	{
	case SIGINT:
	case SIGTERM:
		// follow Bash convention (why not?)
		exit(128 + sig_number);
	default:
		exit(EXIT_FAILURE);
	}
}


// Try to become a direct child of init (PID 1).
// If success, syslog shall be used instead of sdtout/stderr.
void daemonize(void)
{
	printf("launching as a daemon...\n");
	if (daemon(0, 1) != 0)
		// this might be printed with some formatting issues
		fprintf(stderr, "can not run as a daemon: %s\n", strerror(errno));
	else
	{
		Is_daemon = (_Bool)1;
		openlog(PROGNAME, LOG_PID, LOG_DAEMON);
	}
}


// Warn if the same "instance" (same program with same argument) is already running.
void detect_instances(const char *const path)
{
	char check_command[SHELL_COMMAND_MAX_LEN];
	sprintf(
	check_command,
	// there may be better than this
	"ps -Af | grep -v %d | grep -qE \"%s\\s+%s\"", getpid(), PROGNAME, path
	);

#ifndef NDEBUG
	report(LOG_DEBUG, "detecting running instances with `%s`\n", check_command);
#endif

	int rc = system(check_command);
	if (WIFEXITED(rc))
	{
		if (WEXITSTATUS(rc) == 0)
			report(LOG_WARNING, "the same instance is already running!\n");
	}
	else
		report(
		LOG_WARNING,
		"can not check whether the same instance is already running (system() returned code %d)\n",
		rc
		);
}


// Map some commonly used signals to actions.
void register_signals(void)
{
	struct sigaction act = {
		.sa_handler = terminate,
		.sa_flags = 0,
		.sa_restorer = NULL
	};
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
}


// Display a small help on the given stream.
void show_usage(FILE *const s)
{
	fprintf(
	s,
	"\
Usage: %s <path to evdev file, such as /dev/input/eventX>\n\
The program will run as a daemon, unless the environment variable %s is set.\n\
\n",
	PROGNAME, NO_DAEMON_ENVVAR_NAME
	);
}


// Do the initialization and launch the main loop.
int main(const int argc, const char *const argv[])
{
	register_signals();

	if (argc != 2)
	{
		show_usage(stdout);
		terminate(SIGABRT);
	}

	Dev = get_dev(argv[1]);
	if (Dev == NULL)
		terminate(SIGABRT);

	// TODO: replace env var by a command line parameter?
	if (getenv(NO_DAEMON_ENVVAR_NAME) == NULL)
		daemonize();

	detect_instances(argv[1]);

	Config = get_config(CONFPATH);
	if (Critical_error)
		terminate(SIGABRT);
	if (Config)
	{
		int n = 1;
		CONFIG *c = Config;
		while (c->next) n++, c = c->next;
		report(LOG_INFO, "registered %d rules from \"%s\"\n", n, CONFPATH);
	}
	else
		report(LOG_INFO, "running for testing purpose (reporting events values)\n");

	report(LOG_INFO, "now reading: %s\n", libevdev_get_name(Dev));
	int rc;
	do
	{
		// does the polling for us, no need to use poll.h
		rc = libevdev_next_event(
		     Dev,
		     LIBEVDEV_READ_FLAG_NORMAL | LIBEVDEV_READ_FLAG_BLOCKING,
		     &Event
		     );
		if (rc == LIBEVDEV_READ_STATUS_SUCCESS)
			process_event();
	} while (rc == LIBEVDEV_READ_STATUS_SYNC
	      || rc == LIBEVDEV_READ_STATUS_SUCCESS
	      || rc == -EAGAIN);

	report(LOG_ERR, "can not handle event: %s\n", strerror(-rc));
	terminate(SIGABRT);
}
