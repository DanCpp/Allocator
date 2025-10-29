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

  int cap = 2;
  int len = 0;
  int* arr = malloc(2 * sizeof(int));
  int n = 10;
  for (int i = 0; i < n; i++) {
    if (len == cap) {
      cap <<= 1;
      arr = realloc(arr, cap * sizeof(int));
    }

    arr[len++] = i;
  }

  for (int i = 0; i < len; i++) {
    printf("%d - %d\n", i, arr[i]);
  }

  printf("cap: %d\n", cap);

  free(arr);
  return 0;
}