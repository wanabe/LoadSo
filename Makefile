INCDIRS=
LIBDIRS=

A=libloadso.a
LDFLAGS=$(LIBDIRS) -shared -Wl,--out-implib,$(A) -Wl,--export-all-symbols
DLL=msvcrt-ruby191.dll
CFLAGS=-Wall -DDLL_NAME="\"$(DLL)\""
OBJS=class.o string.o variable.o load_so.o st.o
DEFS=load_so.def
LIBS=

all: $(DLL) $(A)

$(DLL) $(A): $(OBJS)
	$(CC) $(LDFLAGS) -o $(DLL) $(OBJS) $(DEFS) $(LIBS)

$(OBJS): rgss.h load_so.h
