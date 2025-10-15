
#define USE_STD_ALLOC

#ifdef USE_STD_ALLOC
#include "std_alloc/std_alloc_wrap.h"
#else
#include "alloc/alloc.h"
#endif

#include <stdlib.h>


int main() {
  #ifndef USE_STD_ALLOC
  create();
  atexit(delete);
  #endif
}