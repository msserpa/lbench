CC=gcc
CFLAGS = -lnuma -lm -fopenmp -Wall -Wextra
OBJ = main.o

all: lbench

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

lbench: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o lbench
