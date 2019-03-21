CC = gcc
LIB = -lpthread
EXE0 = pp1_min

$(EXE0): $(EXE0).exe
	./$<	

%.exe : %.c
	$(CC) $(LIB) -o $@ $^

all: $(EXE0)

clean:
	rm -f *.exe 
