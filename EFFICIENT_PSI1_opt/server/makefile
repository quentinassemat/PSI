LINK_TARGET = \
	server \

OBJS = \
	server.o \
	../../TCP/tcp.o \
	../../profil_opt_hach/profil.o \
	../../profil_opt_hach/hashtable.o \
	../../profil_opt_hach/buffer.o \
	../../profil_opt_hach/base64.o \
	../../profil_opt_hach/random.o \
	../../profil_opt_hach/sha3.o \
	# test.o \

LIB = -I /usr/local/include/pbc  -L /usr/local/lib -Wl,-rpath /usr/local/lib -l pbc -l gmp

REBUILDABLES = $(OBJS) $(LINK_TARGET)

all : $(LINK_TARGET)

clean:
	rm -f $(REBUILDABLES)

$(LINK_TARGET) : $(OBJS)
	cc -g -o $@ $^ $(LIB)

%.o : %.c
	cc -g  -Wall -o $@ -c $<

server.o : server.h server.c ../../TCP/tcp.o ../../profil/profil.o
