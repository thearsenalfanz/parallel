CC = gcc
LIB = -lpthread
EXE0 = gauss

$(EXE0): $(EXE0).exe
	./$< 100 1 0

%.exe : %.c
	$(CC) $(LIB) -o $@ $^

all: $(EXE0)

clean:
	rm -f *.exe 
