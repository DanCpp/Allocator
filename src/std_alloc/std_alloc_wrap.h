#ifndef STD_ALLOC_WRAP
#define STD_ALLOC_WRAP

#include <stddef.h>

void* allocate(size_t nmemb);
void* allocate_filled(size_t n, size_t memb);
void* reallocate(void* old, size_t nmemb);
void deallocate(void* ptr);

#endif