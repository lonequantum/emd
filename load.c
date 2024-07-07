#include "main.h"
extern _Bool Critical_error;


// Return the LIBEVDEV object used with the given evdev filename.
LIBEVDEV *get_dev(const char *const path)
{
	LIBEVDEV *dev;

	int fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		report(LOG_ERR, "can not open \"%s\": %s\n", path, strerror(errno));
		return NULL;
	}

	// TODO: drain pending events, as recommended in libevdev documentation (how?)

	int rc = libevdev_new_from_fd(fd, &dev);
	if (rc < 0)
	{
		report(LOG_ERR, "can not initialize evdev with \"%s\": %s\n", path, strerror(-rc));
		return NULL;
	}

	return dev;
}


// Map the configuration file into memory.
// Set Critical_error in case of allocation failure.
CONFIG *get_config(const char *const confpath)
{
	if (access(confpath, R_OK))
	{
		report(LOG_WARNING, "can not open \"%s\": %s\n", confpath, strerror(errno));
		return NULL;
	}

	char parse_command[SHELL_COMMAND_MAX_LEN];
	sprintf(
	parse_command,
	"awk -F':' '/^\\d+:\\d+:\\d+:.+/ {print $1\"\\n\"$2\"\\n\"$3\"\\n\"$4}' < \"%s\"",
	confpath
	);

#ifndef NDEBUG
	report(LOG_DEBUG, "parsing configuration file with `%s`\n", parse_command);
#endif

	FILE *f = popen(parse_command, "r");
	if (f == NULL)
	{
		report(
		LOG_ERR,
		"can not parse the configuration file (popen() failed)\n"
		);
		return NULL;
	}

	// var c doesn't need init, but compilation may raise warnings if removed
	CONFIG *config = NULL, *c = NULL;

	char line[CONFIG_ITEM_MAX_LEN];
	int n = 0;
	while (fgets(line, CONFIG_ITEM_MAX_LEN, f))
		switch (n++ % 4)
		{
		case 0:
			c = (CONFIG *)malloc(sizeof(CONFIG));
			if (c == NULL)
			{
				report(
				LOG_ERR,
				"can not allocate memory (failed to obtain %d bytes)\n",
				sizeof(CONFIG)
				);
				Critical_error = (_Bool)1;
				return NULL;
			}

			c->type = atoi(line);
			break;
		case 1:
			c->code = atoi(line);
			break;
		case 2:
			c->value = atoi(line);
			break;
		case 3:
			strcpy(c->command, line);
			// remove line break
			c->command[strlen(c->command) - 1] = 0;

			c->next = config;
			config = c;
		}

	int rc = pclose(f);
	if (rc == -1 || !WIFEXITED(rc) || WEXITSTATUS(rc))
		report(
		LOG_WARNING,
		"the configuration file might have been read incorrectly: %s\n",
		rc == -1 ? strerror(errno)
		         : "the parse command could not run or exit normally"
		);

#ifndef NDEBUG
	for (c = config; c; c = c->next)
		report(
		LOG_DEBUG,
		"registered command `%s` for t:c:v %d:%d:%d\n",
		c->command, c->type, c->code, c->value
		);
#endif

	return config;
}
