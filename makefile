BOARD_BASE = 3 # 3, 5, 6, 8

main: main.c init_sudoku.c cell_bit_operations.c
	gcc -O3 -g -o main main.c init_sudoku.c cell_bit_operations.c

clean:
	rm -f main

run: main
	./main $(BOARD_BASE)