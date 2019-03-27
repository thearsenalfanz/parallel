CC = gcc
LIB = -fomp
EXE0 = matrix_print

$(EXE0): $(EXE0).exe
	./$< -N 100 1
	./$< -N 100 2
	./$< -N 100 4
	./$< -N 100 8

%.exe : %.c
	$(CC) $(LIB) -o $@ $^

all: $(EXE0)

clean:
	rm -f *.exe 
