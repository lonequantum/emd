#include "main.h"
extern CONFIG             *Config;
extern struct input_event  Event;


// Try to run something, based on the given input event and the loaded configuration.
// Every detected event goes through this function.
void process_event()
{
	if (Config == NULL)
	{
		static struct timespec tp;
		clock_gettime(CLOCK_REALTIME, &tp);

		report(
		LOG_DEBUG,
		"[%d.%ld] type: %d (%s) - code: %d (%s) - value: %d\n",
		(int)tp.tv_sec, tp.tv_nsec,
		Event.type,
		libevdev_event_type_get_name(Event.type),
		Event.code,
		libevdev_event_code_get_name(Event.type, Event.code),
		Event.value
		);
	}

	for (CONFIG *c = Config; c; c = c->next)
		if (Event.type  == c->type
		 && Event.code  == c->code
		 && Event.value == c->value)
		{
#ifndef NDEBUG
			report(LOG_NOTICE, "running `%s`\n", c->command);
#endif
			int rc = system(c->command);
			if (WIFEXITED(rc))
			{
				if (WEXITSTATUS(rc))
					report(
					LOG_WARNING,
					"command `%s` exited with value %d\n",
					c->command, WEXITSTATUS(rc)
					);
			}
			else
				report(
				LOG_WARNING,
				"command `%s` could not run or exit normally (system() returned code %d)\n",
				c->command, rc
				);
			break;
		}
}
