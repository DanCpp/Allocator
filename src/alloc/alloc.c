#include "alloc.h"

#include <stdlib.h>

#define MEGABYTE (long long int) 1e6

#define DEFAULT 460

#define ARENAS 16
#define DEFAULT_ARENA_BS 8
#define DEFAULT_ARENA_SIZE 10 * MEGABYTE
#define DEFAULT_LARGE_ARENA_SIZE 300 * MEGABYTE

typedef struct __freed__ {
  char* address;
  size_t bytes;

  struct __freed__* next;
} freed;
typedef struct __arena__ {
  char* address;
  char* current;
  size_t BS;
  size_t size;



  freed* first;
} arena;
typedef struct __allocator__ {
  char* memory;
  char * const end_address;
  size_t memory_size;


  arena all_bs[ARENAS];
  arena large;

} allocator;

static allocator* alloc;

static void 
initialize_special_arenas(arena * const arenas, const size_t size, const char * const start_address) {
  for (size_t index = 0; index < ARENAS; index++) {
    arenas[index].BS = DEFAULT_ARENA_BS + DEFAULT_ARENA_BS * index;
    arenas[index].first = NULL;
    arenas[index].size = DEFAULT_ARENA_SIZE;

    arenas[index].address = start_address + DEFAULT_ARENA_SIZE * index;
  }
}

void create() {
  alloc = malloc(sizeof(allocator));
  if (!alloc) {
    abort();
  }

  alloc->memory_size = DEFAULT * MEGABYTE;
  alloc->memory = malloc(alloc->memory_size);
  if (!alloc->memory) {
    abort();
  }
  alloc->end_address = alloc->memory + alloc->memory_size;

  // Initialize special arenas
  initialize_special_arenas(alloc->all_bs, ARENAS, alloc->memory);

  // Initialize large arena
  alloc->large.address = alloc->all_bs[ARENAS - 1].address + alloc->all_bs[ARENAS - 1].size;
  alloc->large.first = malloc(sizeof(*alloc->large.first));
  alloc->large.first->address = alloc->large.address;
  alloc->large.first->bytes = DEFAULT_LARGE_ARENA_SIZE;
  alloc->large.first->next = NULL;
}


void delete() {
  for (freed* next; alloc->large.first; alloc->large.first = next) {
    next = alloc->large.first->next;
    free(alloc->large.first);
  }

  free(alloc->memory);
  free(alloc);
}



void* allocate(size_t nmemb) {
  
}