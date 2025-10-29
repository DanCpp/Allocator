
GCC = gcc
MAIN_FLAGS = -std=c99 -g -O0
WARNINGS_FLAGS = -Wall -Wextra -Wpedantic -Wduplicated-branches -Wduplicated-cond -Wcast-qual -Wconversion -Wsign-conversion -Wlogical-op -Werror
SANITIZER_FLAGS = -fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract -fsanitize=leak -fsanitize=undefined -fsanitize-address-use-after-scope
FLAGS = $(MAIN_FLAGS) $(WARNINGS_FLAGS) $(SANITIZER_FLAGS)

MAIN_SOURCE = src/main.c
ALLOC_SOURCE = src/alloc/alloc.c

TARGET = main

$(TARGET): $(MAIN_SOURCE) $(ALLOC_SOURCE)
	$(GCC) $(FLAGS) $(MAIN_SOURCE) $(ALLOC_SOURCE) -o $@

std: $(MAIN_SOURCE) $(ALLOC_SOURCE)
	$(GCC) -DUSE_STD_ALLOC $(FLAGS) $(MAIN_SOURCE) $(ALLOC_SOURCE) -o $@

all: $(TARGET) std


clean: 
	@rm -f main
	@rm -f std

.PHONY: std all