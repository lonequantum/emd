# EMD
Event Mapping Daemon for systems using evdev

This is a simple daemon C program in less than 400 SLOC that links actions (shell commands) to events managed by [https://www.freedesktop.org/wiki/Software/libevdev/](evdev).  
It's _almost_ POSIX compliant (use of daemon(), vsyslog()) and should compile, in theory, on any Linux system and FreeBSD.  
It should work system-wide, under kernel console mode or X or Wayland or whatever you use.  
It's the very first (alpha) version of something that might be rewritten from scratch some day.

## Author's primary use case
I use [https://alpinelinux.org/](Alpine Linux) on my laptop, mainly in kernel console mode with [https://github.com/tmux/](tmux).  
Even in this mode, I want all my "Fn" keys to be assigned to what their symbols mean. Some are already mapped, some aren't (for example the sound volume keys).  
There are maybe several solutions to adress this problem, but I wanted something simple. I tried [https://github.com/thkala/actkbd](actkbd), but it does not work on my system.  
EMD was born :)

## Build
The only dependency is `libevdev`. Review the Makefile, run `make release` and edit/copy the dist config file to the appropriate location (by default /etc/emd.conf).  
With no configuration found, the program will report the parameters to use in the config file (see below).

## Use
First, the `evdev` module must be loaded.  
Run `emd /dev/input/event<X>` to start monitoring the events reported in `/dev/input/event<X>`.
If you don't use any configuration file (or if it's empty), it will report (via syslog) the *type*, *code* and *value* for each detected event, such as a pressed key. You can get the output directly in your terminal (instead of /var/log/messages) by launching the program in foreground with `NODAEMON=1 emd /dev/…`.  
Otherwise, the dist file shows an example of key-action mapping (one per line). Remember, these actions are shell commands (passed to system()) that may need stream redirections and/or the use of a trailing `&` for non-blocking actions.  
A running instance of EMD works with one evdev input file only (`/dev/input/event<X>`). If the events you want to monitor are managed across multiple files, you have to launch multiple instances of EMD, each with its evdev file as parameter.
