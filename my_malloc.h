#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct info {
  struct info * prev_free;
  struct info * next_free;
  size_t size;

  // int type;
};

typedef struct info inf;

#define s_inf sizeof(inf)
// #define ALLOCATED 1
// #define FREED 0

inf * first_block = NULL;
inf * first_free = NULL;
// inf * last_block = NULL;
// inf * last_free = NULL;

unsigned int get_data_segment_size();
unsigned int get_data_segment_free_space_size();

void * ff_malloc(size_t size);
void ff_free(void * ptr);

void * bf_malloc(size_t size);
void bf_free(void * ptr);

void debug_print();
void debug_block();

inf * expand(size_t s);
void * give_ptr(inf * info);
inf * split(inf * info, size_t s);
void occupy(inf * info);
inf * ff_reuse(size_t s);
void add_free_block(inf * info);
void merge(inf * info);
void _f_free(void * ptr);
