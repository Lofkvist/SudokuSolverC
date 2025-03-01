#include "init_sudoku.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static int size = 0;

/**
 * @brief A brief description of the function.
 *
 * Detailed description of what the function does, if necessary.
 *
 * @param[in] param1 Description of the first input parameter.
 * @param[out] param2 Description of the second output parameter.
 * @return Description of what the function returns.
 */

/*
unit - Any square, row or column
peers - Numbers withing the same unit
base - the square root of the number of peers in a unit
*/

/*
Future improvements
- Set numbers directly in init_sudoku instead of in another functions
- Parallelize
- Change to more optimal types (eg uint_fast8_t)
- Place memory allocations closer together? Both the grid itself and all
pointers to peers
- Improve init_cell_peers? Mutual pointers?
- Vectorize all for loops
- Bigger malloc allocations, memory more contigous
*/

void printBinary(uint_fast64_t num, int len);

Sudoku *init_sudoku(int N);
void free_sudoku(Sudoku *sudoku);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: <BASE>");
        return 1;
    }
    int base = strtoull(argv[1], NULL, 10);
    Sudoku *sudoku = init_sudoku(base);
    int len = sudoku->len;
    Cell **grid = sudoku->grid;

    int i, j;
    for (i = 0; i < len; i++) {
        for (j = 0; j < len; j++) {
            printf("%d ", grid[i][j].value);
        }
        printf("\n");
    }

    free_sudoku(sudoku);

    return 0;
}

void printBinary(uint_fast64_t num, int len) {
    // Print the bits from the highest bit (63) to the lowest bit (0)
    for (int i = len - 1; i >= 0; i--) {
        putchar((num & (1ULL << i)) ? '1' : '0');
    }
    printf(" ");
}