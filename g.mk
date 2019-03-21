CC = gcc
LIB = -lpthread
EXE0 = gauss

$(EXE0): $(EXE0).exe
	./$< 4 1 0
	./$< 4 2 0
	./$< 4 4 0
	./$< 4 8 0

%.exe : %.c
	$(CC) $(LIB) -o $@ $^

all: $(EXE0)

clean:
	rm -f *.exe 
