CC=gcc
LIBS=-lm

all:
        $(CC) Timing.c $(LIBS) -o timing

clean:
        rm timing