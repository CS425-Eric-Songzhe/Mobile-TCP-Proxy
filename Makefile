
FLAGS = gcc -Wall          

.PHONY : all clean client server                                       
all : client server

client : 
	$(FLAGS) client.c -o $@

server : 
	$(FLAGS) server.c -o $@

clean :
	/bin/rm -f *.o *.gcov *.gcno *gcda client server
