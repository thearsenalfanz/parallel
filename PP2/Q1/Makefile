CC = gcc
LIB = -fomp
EXE0 = matrix_inner
EXE1 = matrix_2inner
EXE2 = matrix_all

$(EXE0): $(EXE0).exe
	./$< -N 100 1
	./$< -N 100 2
	./$< -N 100 4
	./$< -N 100 8
$(EXE1): $(EXE1).exe
	./$< -N 100 1
	./$< -N 100 2
	./$< -N 100 4
	./$< -N 100 8
$(EXE2): $(EXE2).exe
	./$< -N 100 1
	./$< -N 100 2
	./$< -N 100 4
	./$< -N 100 8

%.exe : %.c
	$(CC) $(LIB) -o $@ $^

all: $(EXE0) $(EXE1) $(EXE2)

clean:
	rm -f *.exe 
