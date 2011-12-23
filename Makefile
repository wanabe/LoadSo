INCDIRS=
LIBDIRS=

LDFLAGS=-shared -o $(OUT) $(LIBDIRS)
OUT=ext_rgss.dll
OBJS=ext_rgss.o
LIBS=
GAME=./Game.exe
HEADERS=

all: $(OUT)

test: all
	$(GAME)

$(OUT): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS)

$(OBJS): $(HEADERS)
