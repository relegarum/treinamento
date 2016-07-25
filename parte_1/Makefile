
CC=gcc
CFLAGS=-c -Wall -pedantic

all: treinamento_build

treinamento_build: main.o http_utils.o 
    $(CC) main.o http_utils.o -o treinamento

main.o: main.cpp
    $(CC) $(CFLAGS) main.cpp

http_utils.o: factorial.cpp
    $(CC) $(CFLAGS) http_utils.cpp

clean:
    rm *o treinamento
