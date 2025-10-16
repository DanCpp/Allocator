#ifndef USE_STD_ALLOC
#include "alloc/alloc.h"
#endif

#include <stdlib.h>
#include <stdio.h>


int main() {
  #ifndef USE_STD_ALLOC
  create();
  atexit(delete);
  #endif

  int* numbers = allocate(sizeof(int) * 4);

  for (size_t i = 0; i < 4; i++) {
    numbers[i] = (int) i;
  }

  for (size_t i = 0; i < 4; i++) {
    printf("%d\n", numbers[i]);
  }

  free(numbers);
  return 0;
}