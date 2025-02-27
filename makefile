BOARD_BASE = 5

main: main.c init_sudoku.c
	gcc -o main main.c init_sudoku.c

clean:
	rm -f main

run: main
	./main $(BOARD_BASE)
