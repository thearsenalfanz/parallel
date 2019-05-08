CC = mpicc
EXE0 = b
EXE1 = a
EXE2 = gauss_serial

$(EXE0): $(EXE0).exe
	mpirun -np 1 -hostfile hostfile -pernode ./$< 2000 1 0
	mpirun -np 2 -hostfile hostfile -pernode ./$< 2000 2 0
	mpirun -np 4 -hostfile hostfile -pernode ./$< 2000 4 0
	mpirun -np 8 -hostfile hostfile -pernode ./$< 2000 8 0
	mpirun -np 16 -hostfile hostfile -npernode 2 ./$< 2000 16 0
	mpirun -np 32 -hostfile hostfile -npernode 4 ./$< 2000 32 0


$(EXE1): $(EXE1).exe
	mpirun -np 1 -hostfile hostfile -pernode ./$< 2000 1 0
	mpirun -np 2 -hostfile hostfile -pernode ./$< 2000 2 0
	mpirun -np 4 -hostfile hostfile -pernode ./$< 2000 4 0
	mpirun -np 8 -hostfile hostfile -pernode ./$< 2000 8 0
	mpirun -np 16 -hostfile hostfile -npernode 2 ./$< 2000 16 0
	mpirun -np 32 -hostfile hostfile -npernode 4 ./$< 2000 32 0

$(EXE2): $(EXE2).exe
	mpirun -np 1 -hostfile hostfile -pernode ./$< 4 4 0
	
	
%.exe : %.c
	$(CC) -o $@ $^ -lm

all: $(EXE0) $(EXE1) $(EXE2)

clean:
	rm -f *.exe

.PHONY: all $(EXE0) $(EXE1) $(EXE2) clean
