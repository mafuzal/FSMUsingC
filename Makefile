CC=gcc
CFLAGS=-I.
DEPS = generateFSM.h
OBJ=runFSM.o generateFSM.o 
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) 
clean:
	rm -f *.o main 
