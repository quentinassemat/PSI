OBJS = \
	profil.o \
	hashtable.o \
	buffer.o \
	sha3.o \
	random.o \
	base64.o \

all : $(OBJS)

clean:
	rm -f $(OBJS)
	
%.o : %.c
	cc -g  -Wall -o $@ -c $<

profil.o : profil.h profil.c
hashtable.o : hashtable.h hashtable.c
buffer.o : buffer.h buffer.c
sha3.o : sha3.h sha3.c
base64.o : base64.h base64.c