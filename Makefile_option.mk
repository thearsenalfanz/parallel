CC = gcc
LIB = -lpthread
EXE0 = pp1_min_op

$(EXE0): $(EXE0).exe
	./$<	

%.exe : %.c
	$(CC) $(LIB) -o $@ $^

all: $(EXE0) -T 1 $(EXE0) -T 2 $(EXE0) -T 4 $(EXE0) -T 8

clean:
	rm -f *.exe 
