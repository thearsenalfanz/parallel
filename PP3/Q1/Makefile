CC = mpicc
EXE0 = mpi_matrixmul

# mpirun -np 1 -hostfile hostfile -pernode ./mpi_matrixmul.exe 2000
# mpirun -np 2 -hostfile hostfile -pernode ./mpi_matrixmul.exe 2000
# mpirun -np 4 -hostfile hostfile -pernode ./mpi_matrixmul.exe 2000
# mpirun -np 8 -hostfile hostfile -pernode ./mpi_matrixmul.exe 2000
# mpirun -np 16 -hostfile hostfile -npernode 2 ./mpi_matrixmul.exe 2000
# mpirun -np 32 -hostfile hostfile -npernode 4 ./mpi_matrixmul.exe 2000

# $(EXE0): $(RUN) -np  $(EXE0).exe
# 	./$< -N 400 -T 1
# 	./$< -N 400 -T 2
# 	./$< -N 400 -T 4
# 	./$< -N 400 -T 8
%.exe : %.c
	$(CC) -o $@ $^

# all: $(EXE0) $(EXE1) $(EXE2)

clean:
	rm -f *.exe 
