INCDIRS=
LIBDIRS=

LDFLAGS=$(LIBDIRS) -shared
DLL=msvcrt-ruby191.dll
CFLAGS=-Wall -DDLL_NAME="\"$(DLL)\""
OBJS=load_so.o
LIBS=

all: $(DLL)

$(DLL): $(OBJS)
	$(CC) $(LDFLAGS) -o $(DLL) $(OBJS) $(LIBS)

load_so.o: rgss.h load_so.h
