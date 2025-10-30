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

  size_t cap = 2;
  size_t len = 0;
  int* arr = malloc(2 * sizeof(int));
  size_t n = 1000;
  for (size_t i = 0; i < n; i++) {
    if (len == cap) {
      cap <<= 1;
      arr = realloc(arr, cap * sizeof(int));
    }

    arr[len++] = (int) i;
  }

  for (size_t i = 0; i < len; i++) {
    printf("%zu - %d\n", i, arr[i]);
  }

  printf("cap: %zu\n", cap);

  free(arr);
  return 0;
}