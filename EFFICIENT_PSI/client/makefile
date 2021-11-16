LINK_TARGET = \
	client \

OBJS = \
	client.o \
	../../TCP/tcp.o \
	../../profil/profil.o \
	../../profil/hashtable.o \
	# ../../Tools/buffer.o \
	# ../../Tools/random.o \
	# ../../Tools/hashtable.o \
	# ../../Tools/base64.o \
	# ../../Tools/bits.o \

LIB = \

REBUILDABLES = $(OBJS) $(LINK_TARGET)

all : $(LINK_TARGET)

clean:
	rm -f $(REBUILDABLES)

$(LINK_TARGET) : $(OBJS)
	cc -g $(LIB) -o $@ $^ 

%.o : %.c
	cc -g  -Wall -o $@ -c $<

client.o : client.h client.c ../../TCP/tcp.o ../../Tools/buffer.o ../../Tools/random.o ../../Tools/hashtable.o ../../Tools/base64.o ../../Tools/bits.o