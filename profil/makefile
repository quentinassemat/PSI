OBJS = \
	profil.o \
	hashtable.o \

all : $(OBJS)

clean:
	rm -f $(OBJS)
%.o : %.c
	cc -g  -Wall -o $@ -c $<

profil.o : profil.h profil.c
hashtable.o : hashtable.h hashtable.c