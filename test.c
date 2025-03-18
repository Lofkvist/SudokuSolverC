#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int find_first_set_bit(uint_fast64_t candidates, int len);

void test_find_first_set_bit() {
    int num = find_first_set_bit(0b001000000000000000000000000000000000, 36);
    printf("num: %d\n", num);
    // Basic tests
    assert(find_first_set_bit(0b0001, 4) == 1);
    assert(find_first_set_bit(0b0010, 4) == 2);
    assert(find_first_set_bit(0b0100, 4) == 3);
    assert(find_first_set_bit(0b1000, 4) == 4);
    
    // Multiple bits set
    assert(find_first_set_bit(0b1010, 4) == 2);
    assert(find_first_set_bit(0b0110, 4) == 2);
    assert(find_first_set_bit(0b1101, 4) == 1);
    
    // No bits set
    assert(find_first_set_bit(0b0000, 4) == -1);
    
    // Larger length
    assert(find_first_set_bit(0b100000, 6) == 6);
    assert(find_first_set_bit(0b100001, 6) == 1);
    assert(find_first_set_bit(0b000010, 6) == 2);
    
    // Edge case: Single bit length
    assert(find_first_set_bit(0b1, 1) == 1);
    assert(find_first_set_bit(0b0, 1) == -1);
    
    // Higher bits
    assert(find_first_set_bit(1 << 30, 31) == 31);
    assert(find_first_set_bit(1 << 31, 32) == 32);
    assert(find_first_set_bit((1 << 31) | 1, 32) == 1);
    assert(find_first_set_bit(0, 32) == -1);
}

int main() {
    test_find_first_set_bit();
    return 0;
}

int find_first_set_bit(uint_fast64_t candidates, int len) {
    for (int bit = 0; bit < len; bit++) {
        if (candidates & (1ULL << bit)) {
            return bit + 1; // return number within the valid range (1 - len)
        }
    }
    return -1; // In case no valid candidates are left
}