#include <stdlib.h>
#include <stdio.h>

int main() {
    int x, y;
    int br = 1;
    int bc = 1;
    int base = 5;
    int inc = 0;
    for (x = br * base; x < (br + 1) * base; x++) {
        for (y = bc * base; y < (bc + 1) * base; y++) {
            printf("%d ", inc);
            inc++;
        }
        printf("\n");
    }
}