FLAGS = -Wall -pthread
INCLUDES = ../include

all: cpu mem threads io

clean:
			rm -f cpu mem threads io

cpu: cpu.c
			gcc -I $(INCLUDES) -o cpu cpu.c $(FLAGS)

mem: mem.c 
			gcc -I $(INCLUDES) -o mem mem.c $(FLAGS)

threads:	threads.c
			gcc -I $(INCLUDES) -o threads threads.c $(FLAGS)

io:	 io.c
			gcc -I $(INCLUDES) -o io io.c $(FLAGS)
