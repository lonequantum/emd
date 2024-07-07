progname = emd
confpath = /etc/$(progname).conf

compile = c99 *.c *.h -o $(progname) -Wall\
          -I /usr/include/libevdev-1.0/libevdev/ -levdev\
          -D_DEFAULT_SOURCE\
          -DPROGNAME=\"$(progname)\"\
          -DCONFPATH=\"$(confpath)\"

default:
	$(compile) -g
release:
	$(compile) -Os -DNDEBUG
	strip $(progname)
