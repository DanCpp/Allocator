#include "alloc.h"

#undef malloc
#undef realloc
#undef calloc
#undef free

#include <stdint.h>
#include <stdio.h>

#define PTR_SIZE sizeof(char*)
#define INT_SIZE sizeof(uint32_t)
#define MEGABYTE (1 << 20)
#define MIN_SIZE 8
#define MAX_SIZE 128
#define SPECIAL_ARENAS 16

#define min(a, b) (a < b) ? a : b

#include <stdlib.h>
#include <string.h>

void* nc_malloc(size_t nmemb) {
  void* ptr = malloc(nmemb);
  if (!ptr) {
    abort();
  }

  return ptr;
}

typedef struct {
  char* memory;
  char** head;
  uint32_t bs;
  size_t size;
}special_arena_t;

typedef struct {
  char* memory;
  special_arena_t s_arenas[SPECIAL_ARENAS];
}allocator_t;

static allocator_t allocator;

static void initialize_specials() {
  uint32_t bs_next = MIN_SIZE;
  for (size_t index = 0; index < SPECIAL_ARENAS; index++, bs_next += 8) {
    special_arena_t* arena = &allocator.s_arenas[index];
    arena->bs = bs_next;
    arena->size = 10 * MEGABYTE / (arena->bs + PTR_SIZE) * (arena->bs + PTR_SIZE);
    if (index)
      arena->memory = allocator.s_arenas[index - 1].memory + 10 * MEGABYTE;
    else
      arena->memory = allocator.memory;

    arena->head = (char**)arena->memory;
    char** next = arena->head;
    char** prev = NULL;
    while ((char*)next < arena->memory + arena->size) {
      if (prev)
        *next = *prev + PTR_SIZE + arena->bs;
      else
        *next = (char*)arena->head + PTR_SIZE + arena->bs;

      prev = next;
      next = (char**)(*next);
    }
    *prev = NULL;
    if (bs_next == 40) {
      printf("%p\n", *arena->head);
    }
    ///
  }
}

void create() {
  allocator.memory = nc_malloc(460 * MEGABYTE);
  initialize_specials();
}

void delete() {
  free(allocator.memory);
}

static void* allocate_in_specials(size_t nmemb) {
  size_t round_to_eight = (nmemb / 8 + (nmemb % 8 ? 1 : 0)) * 8;
  special_arena_t* allocation_arena = &allocator.s_arenas[(round_to_eight - 8) / 8];
  if (*allocation_arena->head == NULL)
    return NULL;

  char** new_head = (char**)(*allocation_arena->head);
  *(uint32_t*)allocation_arena->head = allocation_arena->bs;  
  void* ptr = ((char*)allocation_arena->head + PTR_SIZE);

  allocation_arena->head = new_head;
  return ptr;
}

static uint32_t get_size(void* ptr) {
  return *(uint32_t*)((char*)ptr - PTR_SIZE);
}

void* allocate(size_t nmemb) {
  return allocate_in_specials(nmemb);
}

void* allocate_filled(size_t n, size_t memb) {
  void* ptr = allocate(n * memb);
  if (!ptr)
    return NULL;

  memset(ptr, 0, n * memb);
  return ptr;
}

void* reallocate(void* old, size_t nmemb) {
  char* c_new = allocate(nmemb);
  if (!c_new)
    return NULL;


  char* c_old = (char*)old;
  uint32_t new_copied_block = min(get_size(old), (uint32_t) nmemb); 

  memmove(c_new, c_old, new_copied_block);

  deallocate(old);
  return c_new;
}

static void deallocate_specials(void* ptr, size_t ptr_size) {
  special_arena_t* arena = &allocator.s_arenas[(ptr_size - 8) / 8];
  
  char** c_ptr = (char**)((char*)ptr - PTR_SIZE);
  *c_ptr = (char*)arena->head;
  arena->head = c_ptr;
}

void deallocate(void* ptr) {
  size_t ptr_size = get_size(ptr);
  // deallocation in large happened
  if (ptr_size > 128)
    return;

  deallocate_specials(ptr, ptr_size);
}