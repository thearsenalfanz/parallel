CC = gcc
LIB = -lpthread
EXE0 = gauss_test
EXE1 = gauss

$(EXE0): $(EXE0).exe
	./$< 
$(EXE1): $(EXE1).exe
	./$< 

%.exe : %.c
	$(CC) $(LIB) -o $@ $^ -w

all: $(EXE0) $(EXE1)

clean:
	rm -f *.exe 

