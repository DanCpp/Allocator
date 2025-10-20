#include "alloc.h"

#undef malloc
#undef calloc
#undef realloc
#undef free

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MEGABYTE 1048576
#define SPECIAL_ARENA_SIZE 10 * MEGABYTE
#define LARGE_ARENA_SIZE 300 * MEGABYTE
#define ALLOCATOR_MEMORY 460 * MEGABYTE

#define MIN_SIZE 8
#define MAX_SIZE 128
#define NUM_OF_ARENAS 16

void* nc_malloc(size_t nmemb) {
  void* ptr = malloc(nmemb);
  if (!ptr) {
    abort();
  }

  return ptr;
}

typedef struct __block__{
  char* memory_address;
  struct __block__* next;
}block;
typedef struct {
  char* memory_start;
  char* memory_end;
  char* current;
  block* freed;
  size_t bs;
  size_t memory_free;
}arena;

typedef struct __sized__block__ {
  char* memory_address;
  size_t block_size;
  struct __sized__block__* next;
}large_block;

// DONT FORGET:
/*
  DO THE CONCATINATION TWO OF NEIGHBOUR FREED BLOCKS
*/
typedef struct {
  char* memory_start;
  char* memory_end;
  large_block* free;
  large_block* used;
  size_t memory_free;
}large_arena;

typedef struct {
  char* memory;
  arena specials[NUM_OF_ARENAS];
  large_arena large;
}allocator;

static allocator alloc;

static void init_specials(arena* specials) {
  for (size_t index = 0; index < NUM_OF_ARENAS; index++) {
    specials[index] = (arena) {
      .current = alloc.memory + SPECIAL_ARENA_SIZE * index,
      .memory_start = alloc.memory + SPECIAL_ARENA_SIZE * index,
      .bs = MIN_SIZE * (index + 1),
      .freed = NULL,
      .memory_free = SPECIAL_ARENA_SIZE
    };

    specials[index].memory_end = specials[index].memory_start + specials[index].memory_free;
  }
}

static void init_large(large_arena* large) {
  *large = (large_arena) {
    .memory_start = alloc.memory + NUM_OF_ARENAS * SPECIAL_ARENA_SIZE,
    .memory_free = LARGE_ARENA_SIZE,
    .used = NULL,
    .free = nc_malloc(sizeof(large_block)),
  };
  *large->free = (large_block) {
    .block_size = large->memory_free,
    .memory_address = large->memory_start,
    .next = NULL,
  };
  large->memory_end = large->memory_start + large->memory_free;
}

void create() {
  alloc.memory = nc_malloc(ALLOCATOR_MEMORY);
  init_specials(alloc.specials);
  init_large(&alloc.large);
}

static void delete_blocks(arena special) {
  for (block* next; special.freed; special.freed = next) {
    next = special.freed->next;
    free(special.freed);
  }
}

static void delete_large_blocks(large_arena large) {
  for (large_block* next; large.used; large.used = next) {
    next = large.used->next;
    free(large.used);
  }

  for (large_block* next; large.free; large.free = next) {
    next = large.free->next;
    free(large.free);
  }
}

void delete() {
  for (size_t index = 0; index < NUM_OF_ARENAS; index++) {
    delete_blocks(alloc.specials[index]);
  }
  delete_large_blocks(alloc.large);
  free(alloc.memory);
}

static arena* where_allocate(size_t nmemb) {
  assert(nmemb <= MAX_SIZE);

  arena* it = alloc.specials;
  while (it->bs < nmemb && it < alloc.specials + NUM_OF_ARENAS) {
    it++;
  }

  return it;
}

static void* allocate_in_special(size_t nmemb) {
  arena* arena_to_alloc = where_allocate(nmemb);
  if (!arena_to_alloc->memory_free)
    return NULL;

  arena_to_alloc->memory_free -= arena_to_alloc->bs;
  
  if (arena_to_alloc->freed) {
    void* to_return = arena_to_alloc->freed->memory_address;
    block* next = arena_to_alloc->freed->next;
    free(arena_to_alloc->freed);
    arena_to_alloc->freed = next;

    return to_return;
  }

  void* to_return = arena_to_alloc->current;
  arena_to_alloc->current += arena_to_alloc->bs;

  return to_return;
}

static void* allocate_in_large(size_t nmemb) {

  // find first to emplace
  large_block* first = alloc.large.free;
  large_block* prev = NULL;
  while (first && first->block_size < nmemb) {
    prev = first;
    first = first->next;
  }

  if (!first)
    return NULL;

  large_block* new_used = nc_malloc(sizeof(large_block));
  new_used->block_size = nmemb;
  new_used->memory_address = first->memory_address;
  new_used->next = alloc.large.used;
  alloc.large.used = new_used;


  first->block_size -= nmemb;
  first->memory_address += nmemb;

  if (first->block_size == 0) {
    // this is head
    if (!prev) {
      alloc.large.free = first->next;
    } else {
      prev->next = first->next;
    }


    free(first);
  }

  return new_used->memory_address;
}

void* allocate(size_t nmemb) {
  // avoid nmemb = 0
  if (nmemb == 0)
    return NULL;

  void* ptr;
  if (nmemb > MAX_SIZE) {
    ptr = allocate_in_large(nmemb);
  } else {
    ptr = allocate_in_special(nmemb);
    if (!ptr)
      ptr = allocate_in_large(nmemb);
  }

  return ptr;
}


void* allocate_filled(size_t n, size_t memb) {
  void* ptr = allocate(n * memb);
  if (!ptr)
    return NULL;

  memset(ptr, 0, n * memb);
  return ptr;
}

void* reallocate(void* old, size_t nmemb) {
  void* ptr = allocate(nmemb);
  if (!ptr)
    return NULL;
  
  memcpy(ptr, old, nmemb);
  deallocate(old);
  return ptr;
}

static bool in_arena(arena special, void* ptr) {
  return (special.memory_start <= (char*) ptr && (char*) ptr < special.memory_end);
}

static arena* where_was_allocated(void* ptr) {
  arena* it = alloc.specials;
  while (!in_arena(*it, ptr) && it < alloc.specials + NUM_OF_ARENAS)
    it++;

  if (it == alloc.specials + NUM_OF_ARENAS)
    return NULL;

  return it;
}

static void deallocate_in_special(arena* dealloc_arena, void* ptr) {
  block* new_next = dealloc_arena->freed;
  block* new_head = nc_malloc(sizeof(block));

  new_head->memory_address = ptr;
  new_head->next = new_next;

  dealloc_arena->freed = new_head;
}

static void deallocate_in_large(void* ptr) {
  assert(alloc.large.used);
  large_block* find_used = alloc.large.used;
  large_block* prev;
  while (ptr != find_used->memory_address) {
    prev = find_used;
    find_used = find_used->next;
  }

  if (prev)
    prev->next = find_used->next;

  large_block* new_next = alloc.large.freed;
  find_used->next = new_next;
  alloc.large.freed = find_used;
}

void deallocate(void* ptr) {
  arena* alloc_arena = where_was_allocated(ptr);

  if (!alloc_arena) {
    deallocate_in_large(ptr);
  } else {
    deallocate_in_special(alloc_arena, ptr);
  }
}