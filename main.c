#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int size = 0;

/*
unit - Any square, row or column
peers - Numbers withing the same unit
base - the square root of the number of peers in a unit
*/

int init_board(unsigned char *board, const int size);

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: <BASE>");
    return 1;
  }

  size = atoi(argv[1]);
  unsigned char *board = malloc((size * size) * sizeof(unsigned char *));

  uint64_t a = 3000;
  a = handle_number(a);
  printf("a = %ld\n", a);
  printf("size = %ld\n", sizeof(a));

  if (init_board(board, size) != 0) {
    printf("Failed to initialize board\n");
    return 1;
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      printf("%d ", board[i * size + j]);
    }
    printf("\n");
  }

  free(board);
  return 0;
}

int init_board(unsigned char *board, const int size) {
  char filename[50]; // Adjust size as necessary

  // Create the filename
  snprintf(filename, sizeof(filename), "boards/board_%dx%d.dat", size, size);

  FILE *file = fopen(filename, "rb");
  if (!file) {
    perror("Error opening file");
    return 1;
  }

  int num;
  int i = 0;
  while (fread(&num, sizeof(unsigned char), 1, file) == 1) {
    board[i] = num;
    i++;
  }
  fclose(file);
  return 0;
}