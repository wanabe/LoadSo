INCDIRS=
LIBDIRS=

LDFLAGS=$(LIBDIRS) -shared
EXTDLL=ext_rgss.dll
CFLAGS=-Wall -DDLL_NAME="\"$(EXTDLL)\""
OBJS=ext_rgss.o graphics.o
LIBS=d3dx9.lib -ld3d9 -lole32
GAME=./Game.exe
HEADERS=ext_rgss.h rgss.h

all: $(EXTDLL)

test: all
	$(GAME)

$(EXTDLL): $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXTDLL) $(OBJS) $(LIBS)

ext_rgss.o: rgss.h ext_rgss.h graphics.h
graphics.o: rgss.h ext_rgss.h graphics.h
