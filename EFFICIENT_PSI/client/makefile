LINK_TARGET = \
	client \

OBJS = \
	client.o \
	../../TCP/tcp.o \

LIB = \

REBUILDABLES = $(OBJS) $(LINK_TARGET)

all : $(LINK_TARGET)

clean:
	rm -f $(REBUILDABLES)

$(LINK_TARGET) : $(OBJS)
	cc -g $(LIB) -o $@ $^ 

# Here is a Pattern Rule, often used for compile-line.
# It says how to create a file with a .o suffix, given a file with a .c suffix.
# The rule's command uses some built-in Make Macros:
# $@ for the pattern-matched target
# $lt; for the pattern-matched dependency

%.o : %.c
	cc -g  -Wall -o $@ -c $<

#dependecy rule à mettre (.o : .h)
client.o : client.h client.c ../../TCP/tcp.o