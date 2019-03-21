CC = gcc
LIB = -lpthread
EXE0 = pp1_min.c

$(EXE0): $(EXE0).exe
	./$<	

$(EXE1): $(EXE1).exe 
	./$<	

$(EXE2): $(EXE2).exe
	./$<	

$(EXE3): $(EXE3).exe 
	./$<	

%.exe : %.c
	$(CC) $(LIB) -o $@ $^

all: $(EXE0) $(EXE1) $(EXE2) $(EXE3)

clean:
	rm -f *.exe 
