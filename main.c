#include "init_sudoku.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static int size = 0;

/*
unit - Any square, row or column
peers - Numbers withing the same unit
base - the square root of the number of peers in a unit
*/

/*
Future improvements
- Set numbers directly in init_sudoku instead of in another functions
*/

Sudoku *init_sudoku(uint_fast8_t N);
void free_sudoku(Sudoku *sudoku);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: <BASE>");
        return 1;
    }
    uint_fast64_t base = strtoull(argv[1], NULL, 10);
    Sudoku *sudoku = init_sudoku(base);
    printf("len: %hhu\n", sudoku->len);
    printf("base: %hhu\n", sudoku->base);

    int i, j;
    for (i = 0; i < sudoku->len; i++) {
        for (j = 0; j < sudoku->len; j++) {
            printf("%hhu ", sudoku->grid[i][j].value);
        }
        printf("\n");
    }

    free_sudoku(sudoku);

    return 0;
}