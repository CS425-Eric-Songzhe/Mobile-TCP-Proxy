
FLAGS = gcc -Wall          

OTHERS = mysockets.c mymessages.c

.PHONY : all clean cproxy sproxy

all : cproxy sproxy

cproxy :
	$(FLAGS) cproxy.c $(OTHERS) -o cproxy

sproxy :
	$(FLAGS) sproxy.c $(OTHERS) -o sproxy

clean :
	/bin/rm -f *.o *.gcov *.gcno *gcda cproxy sproxy
