LIB = WITH_STDLIB
FOM = NO_USE_COMPLETE_FORMULAS

LINK_TARGET = \
	client \

OBJS = \
	client.o \
	../../TCP/tcp.o \
	../../profil/profil.o \
	../../profil/hashtable.o \
	../../profil/buffer.o \
	../../profil/base64.o \
	../../profil/random.o \
	../../profil/sha3.o \
	test.o \

LIB = -I /usr/local/include/pbc  -L /usr/local/lib -Wl,-rpath /usr/local/lib -l pbc -l gmp

REBUILDABLES = $(OBJS) $(LINK_TARGET)

all : $(LINK_TARGET)

clean:
	rm -f $(REBUILDABLES)

$(LINK_TARGET) : $(OBJS)
	cc -g -o $@ $^ $(LIB)

%.o : %.c
	cc -g  -Wall -o $@ -c $<


client.o : client.h client.c test.o test.c test.h