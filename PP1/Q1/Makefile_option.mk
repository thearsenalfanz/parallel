CC = gcc
LIB = -lpthread
EXE0 = pp1_min_op

$(EXE0): $(EXE0).exe
	./$<	-T 1
	./$<	-T 2
	./$<	-T 4
	./$<	-T 8

%.exe : %.c
	$(CC) $(LIB) -o $@ $^

all: $(EXE0)

clean:
	rm -f *.exe 
