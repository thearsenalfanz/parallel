CC = gcc
LIB = -lpthread
EXE0 = gauss_test

$(EXE0): $(EXE0).exe
	./$< 

%.exe : %.c
	$(CC) $(LIB) -o $@ $^ -w

all: $(EXE0)

clean:
	rm -f *.exe 
