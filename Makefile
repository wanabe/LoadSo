INCDIRS=
LIBDIRS=

LDFLAGS=-shared -o $(OUT) $(LIBDIRS)
OUT=ext_rgss.dll
OBJS=ext_rgss.o
LIBS=-ld3d9 -lole32
GAME=./Game.exe
HEADERS=ext_rgss.h rgss.h

all: $(OUT)

test: all
	$(GAME)

$(OUT): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJS): $(HEADERS)
