#ifndef USE_STD_ALLOC
#include "alloc/alloc.h"
#endif

#include <stdlib.h>
#include <stdio.h>


int main() {
  #ifndef USE_STD_ALLOC
  printf("use my allocator\n");
  create();
  atexit(delete);
  #endif

  int (*matrix)[3] = malloc(3 * 3 * sizeof(int));
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      matrix[i][j] = (i + 1) * (j + 1);
      printf("%d * %d = %d\n", i + 1, j + 1, matrix[i][j]);
    }
  }

  free(matrix);
  return 0;
}