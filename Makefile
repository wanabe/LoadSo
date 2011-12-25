INCDIRS=
LIBDIRS=

A=libloadso.a
LDFLAGS=$(LIBDIRS) -shared -Wl,--out-implib,$(A)
DLL=msvcrt-ruby191.dll
CFLAGS=-Wall -DDLL_NAME="\"$(DLL)\""
OBJS=load_so.o st.o
LIBS=

all: $(DLL) $(A)

$(DLL) $(A): $(OBJS)
	$(CC) $(LDFLAGS) -o $(DLL) $(OBJS) $(LIBS)

load_so.o: rgss.h load_so.h
