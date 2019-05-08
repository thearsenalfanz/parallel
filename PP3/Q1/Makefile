CC = mpicc
EXE0 = mpi_matrixmul

$(EXE0): $(EXE0).exe
	mpirun -np 1 -hostfile hostfile -pernode ./$< 2000
	mpirun -np 2 -hostfile hostfile -pernode ./$< 2000
	mpirun -np 4 -hostfile hostfile -pernode ./$< 2000
	mpirun -np 8 -hostfile hostfile -pernode ./$< 2000
	mpirun -np 16 -hostfile hostfile -npernode 2 ./$< 2000
	mpirun -np 32 -hostfile hostfile -npernode 4 ./$< 2000

	
%.exe : %.c
	$(CC) -o $@ $^ -lm

all: $(EXE0)

clean:
	rm -f *.exe

.PHONY: all $(EXE0) clean