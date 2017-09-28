
FLAGS = gcc -Wall          

.PHONY : all clean cproxy sproxy                                       
all : cproxy sproxy

cproxy : 
	$(FLAGS) cproxy.c -o $@

sproxy : 
	$(FLAGS) sproxy.c -o $@

clean :
	/bin/rm -f *.o *.gcov *.gcno *gcda cproxy sproxy
