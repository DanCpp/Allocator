#include "std_alloc_wrap.h"

#include <stdlib.h>

void* allocate(size_t nmemb) {
  return malloc(nemmb);
}

void* allocate_filled(size_t n, size_t memb) {
  return calloc(n, memb);
}

void* reallocate(void* old, size_t nmemb) {
  return realloc(old, nmemb);
}

void deallocate(void* ptr) {
  free(ptr);
}