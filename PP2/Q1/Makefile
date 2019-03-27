CC = gcc
LIB = -fopenmp
EXE0 = matrix_inner
EXE1 = matrix_2inner
EXE2 = matrix_all

$(EXE0): $(EXE0).exe
	./$< -N 400 -T 1
	./$< -N 400 -T 2
	./$< -N 400 -T 4
	./$< -N 400 -T 8
$(EXE1): $(EXE1).exe
	./$< -N 400 -T 1
	./$< -N 400 -T 2
	./$< -N 400 -T 4
	./$< -N 400 -T 8
$(EXE2): $(EXE2).exe
	./$< -N 400 -T 1
	./$< -N 400 -T 2
	./$< -N 400 -T 4
	./$< -N 400 -T 8

%.exe : %.c
	$(CC) $(LIB) -o $@ $^

all: $(EXE0) $(EXE1) $(EXE2)

clean:
	rm -f *.exe 
