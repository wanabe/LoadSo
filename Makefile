INCDIRS=
LIBDIRS=

LDFLAGS=$(LIBDIRS) -shared
EXTDLL=ext_rgss.dll
CFLAGS=-DDLL_NAME="\"$(EXTDLL)\""
OBJS=ext_rgss.o
LIBS=-ld3d9 -lole32
GAME=./Game.exe
HEADERS=ext_rgss.h rgss.h

all: $(EXTDLL)

test: all
	$(GAME)

$(EXTDLL): $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXTDLL) $(OBJS) $(LIBS)

$(OBJS): $(HEADERS)
