#ifndef ALLOC
#define ALLOC

#include <stddef.h>

struct __allocator__;
typedef struct __allocator__ allocator;

allocator* create();
void delete(allocator* alloc);

void* allocate(size_t nmemb);
void* allocate_filled(size_t n, size_t memb);
void* reallocate(void* old, size_t nmemb);
void deallocate(void* ptr);

#endif