#include <stdint.h>
#include <stdio.h>

void printBinary(uint_fast64_t num) {
    // Print the bits from the highest bit (63) to the lowest bit (0)
    for (int i = 63; i >= 0; i--) {
        putchar((num & (1ULL << i)) ? '1' : '0');
    }
    printf("\n");
}

int main() {
    uint_fast64_t num = 0xFFFFFFFFFFFFFFFF; // Example: all bits set
    int val = 5;
    num &= ~(1ULL << (val-1));                   

    printf("%lx\n", num); // Print in hexadecimal
    printBinary(num);
    return 0;
}