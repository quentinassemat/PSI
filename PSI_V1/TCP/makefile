OBJS = \
	tcp.o \

all : $(OBJS)

clean:
	rm -f $(OBJS)

# Here is a Pattern Rule, often used for compile-line.
# It says how to create a file with a .o suffix, given a file with a .c suffix.
# The rule's command uses some built-in Make Macros:
# $@ for the pattern-matched target
# $lt; for the pattern-matched dependency

%.o : %.c
	cc -g  -Wall -o $@ -c $<

#dependecy rule à mettre (.o : .h)
tcp.o : tcp.h tcp.c