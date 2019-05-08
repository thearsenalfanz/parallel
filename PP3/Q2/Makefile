CC = mpicc
EXE0 = a
EXE1 = b

# mpirun -np 1 -hostfile hostfile -pernode ./a.exe 2000
# mpirun -np 2 -hostfile hostfile -pernode ./a.exe 2000
# mpirun -np 4 -hostfile hostfile -pernode ./a.exe 2000
# mpirun -np 8 -hostfile hostfile -pernode ./a.exe 2000
# mpirun -np 16 -hostfile hostfile -npernode 2 ./a.exe 2000
# mpirun -np 32 -hostfile hostfile -npernode 4 ./a.exe 2000

# mpirun -np 1 -hostfile hostfile -pernode ./b.exe 2000
# mpirun -np 2 -hostfile hostfile -pernode ./b.exe 2000
# mpirun -np 4 -hostfile hostfile -pernode ./b.exe 2000
# mpirun -np 8 -hostfile hostfile -pernode ./b.exe 2000
# mpirun -np 16 -hostfile hostfile -npernode 2 ./b.exe 2000
# mpirun -np 32 -hostfile hostfile -npernode 4 ./b.exe 2000

$(EXE0): $(EXE0).exe
	./$< -N 400 -T 1
	./$< -N 400 -T 2
	./$< -N 400 -T 4
	./$< -N 400 -T 8
%.exe : %.c
	$(CC) -o $@ $^

# all: $(EXE0) $(EXE1) $(EXE2)

clean:
	rm -f *.exe 
