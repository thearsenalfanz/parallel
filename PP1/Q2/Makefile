CC = gcc
LIB = -lpthread
EXE0 = gauss

$(EXE0): $(EXE0).exe
	./$< 1000 8 0 

%.exe : %.c
	$(CC) $(LIB) -o $@ $^ -w

all: $(EXE0) $(EXE1)

clean:
	rm -f *.exe 

