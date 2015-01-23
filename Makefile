CC=gcc

INC=-I.

CFLAGS= -Wall #-g

%.o: %.c $(INC)
	$(CC) -c -o $@ $< $(CFLAGS) $(INC) $(LIBS)
all: turing_sim.o
	$(CC) turing_sim.o
clean:
	rm *.o *.out
