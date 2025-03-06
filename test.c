#include <stdint.h>
#include <stdio.h>
#include "cell_bit_operations.h"

void printBinary(uint_fast64_t num) {
    // Print the bits from the highest bit (63) to the lowest bit (0)
    for (int i = 63; i >= 0; i--) {
        putchar((num & (1ULL << i)) ? '1' : '0');
    }
    printf("\n");
}

int main() {
    uint_fast64_t num = 0; // Example: all bits set
    int val = 5;
    num |= (1ULL << (val-1));
    val = 15;
    num |= (1ULL << (val-1));                   

    printBinary(num);
    int i = 1;
    int pos = find_iths_set_bit(num, i);
    
    

    printf("pos: %d\n", pos);

    //find_iths_set_bit();
    return 0;
}