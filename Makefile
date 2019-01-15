CC=mpicc

all: test-mpi

test-mpi: test-mpi.o
        $(CC) -o test-mpi test-mpi.o

test-mpi.o: test-mpi.c
        $(CC) -o test-mpi.o -c test-mpi.c

clean:
        rm -f test-mpi.o test-mpi