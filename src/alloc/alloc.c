#include "alloc.h"



struct __allocator__ {
  char* memory;
  char** large_arena;
  char** all_mini_arenas;

  size_t bytes_in_large;
  size_t bytes_in_mini;
};

