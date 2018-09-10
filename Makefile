CC ?= gcc
#CFLAGS ?= -std=gnu99 -Werror -Wfatal-errors -Wall -Wextra -pedantic -Og -g -ggdb -pthread -D DEBUG= -fsanitize=address -fsanitize=leak -fsanitize=undefined
CFLAGS ?= -O3 -std=gnu99 -pthread

#Definition Phony
.PHONY: all clean test

#Target
error.o: error.c
	$(CC) $(CFLAGS) -c $< -o $@ -lm

list_data.o: list_data.c error.o
	$(CC) $(CFLAGS) -c $< -o $@ -lm

input.o: input.c list_data.o
	$(CC) $(CFLAGS) -c $< -o $@ -lm

option.o: option.c
	$(CC) $(CFLAGS) -c $< -o $@ -lm

scheduler.o: scheduler.c
	$(CC) $(CFLAGS) -c $< -o $@ -lm

simulator: simulator.c scheduler.o list_data.o input.o option.o error.o
	$(CC) $(CFLAGS) $< -o $@ -lm


#Phony
all: error.o list_data.o input.o option.o simulator

test1: all
	./simulator -op out_pree_1.csv -on out_no_pree_1.csv -i /Inputfile/01_tasks.csv

test2: all
	./simulator -op out_pree_2.csv -on out_no_pree_2.csv -i /Inputfile/02_tasks.csv

test3: all
	./simulator -op out_pree_3.csv -on out_no_pree_3.csv -i /Inputfile/03_tasks.csv

test4: all
	./simulator -op out_pree_4.csv -on out_no_pree_4.csv -i /Inputfile/04_tasks.csv

test5: all
	./simulator -op out_pree_5.csv -on out_no_pree_5.csv -i /Inputfile/05_tasks.csv

test: test1 test2 test3 test4 test5

clean:
	rm -f *.o
	rm -f simulator
	rm -f *.csv
	