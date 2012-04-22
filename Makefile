INCDIRS=
LIBDIRS=

A=libloadso.a
LDFLAGS=$(LIBDIRS) -shared -Wl,--out-implib,$(A) -Wl,--export-all-symbols
DLL=msvcrt-ruby191.dll
CONSTS=
CFLAGS=-Wall -DDLL_NAME="\"$(DLL)\"" $(CONSTS)
OBJS=array.o class.o eval.o hash.o load_so.o numeric.o objspace.o st.o string.o thread.o variable.o
DEFS=load_so.def
LIBS=

all: $(DLL) $(A)

$(DLL) $(A): $(OBJS)
	$(CC) $(LDFLAGS) -o $(DLL) $(OBJS) $(DEFS) $(LIBS)

$(OBJS): rgss.h load_so.h
