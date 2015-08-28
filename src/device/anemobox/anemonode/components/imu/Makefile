CFLAGS=-O2 -Wall
CC=gcc
EXEC_NAME=app_bno055
LDFLAGS=-lmraa
IN=app_bno.c bno055.c 

.PHONY:clean

all:$(EXEC_NAME)

$(EXEC_NAME):
	$(CC) $(CFLAGS) $(IN)$(LDFLAGS) -o $(EXEC_NAME) 
	
clean:
	rm -rf *.o $(EXEC_NAME)
