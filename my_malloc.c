#include "my_malloc.h"

// expand heap with sbrk.
// return NULL if sbrk() fails.
// update last_block and first_block.

inf * expand(size_t s) {
  size_t size = s + s_inf;
  inf * info = (inf *)sbrk(size);

  if (info == (inf *)(-1)) {
    return NULL;
  }

  if (first_block == NULL) {
    first_block = info;
  }
  // last_block = info;
  info->size = size;
  // info->type = ALLOCATED;
  info->prev_free = NULL;
  info->next_free = NULL;
  return info;
}

// give the void * ptr represented by inf * info.

void * give_ptr(inf * info) {
  void * ptr = (void *)((char *)info + s_inf);
  return ptr;
}

// split the free block from behind, return the allocated block.
// do not check if this block is large enough.
// input s should include s_inf.

inf * split(inf * info, size_t s) {
  info->size -= s;
  inf * new = (inf *)((char *)info + info->size);
  new->size = s;
  new->prev_free = NULL;
  new->next_free = NULL;
  // new->type = ALLOCATED;
  return new;
}

// mark entire free block as ALLOCATED, remove from link list.

void occupy(inf * info) {
  if (info->prev_free != NULL) {
    info->prev_free->next_free = info->next_free;
  }
  if (info->next_free != NULL) {
    info->next_free->prev_free = info->prev_free;
  }
  if (info == first_free) {
    first_free = info->next_free;
  }
  info->prev_free = NULL;
  info->next_free = NULL;
  // info->type = ALLOCATED;
}

// find free block with bf logic.

inf * bf_reuse(size_t s) {
  s += s_inf;
  inf * it = first_free;
  size_t b_size = ULONG_MAX;
  inf * b_inf = NULL;

  while (it != NULL) {
    if (it->size >= s && it->size < b_size) {
      b_size = it->size;
      b_inf = it;
    }
    if (b_size == s) {
      break;
    }
    it = it->next_free;
  }

  if (b_inf != NULL) {
    if (b_size - s <= s_inf) {
      occupy(b_inf);
      return b_inf;
    }
    else if (b_size - s > s_inf) {
      inf * new = split(b_inf, s);
      return new;
    }
  }
  return NULL;
}

void * bf_malloc(size_t size) {
  void * ptr;
  inf * target = bf_reuse(size);
  if (target == NULL) {
    target = expand(size);
  }
  ptr = give_ptr(target);
  return ptr;
}

// add info into the link list.

void add_free_block(inf * info) {
  if (first_free == NULL) {
    first_free = info;
    // last_free = info;
  }
  else {
    inf * it = first_free;

    if (it > info) {
      info->next_free = it;
      it->prev_free = info;
      first_free = info;
      return;
    }

    while (it != NULL) {
      if (it->next_free != NULL) {
        if (it->next_free > info) {
          it->next_free->prev_free = info;
          info->next_free = it->next_free;
          it->next_free = info;
          info->prev_free = it;
          return;
        }
      }
      else {
        it->next_free = info;
        info->prev_free = it;
        // last_free = it;

        return;
      }
      it = it->next_free;
    }
  }
  return;
}

// merge blocks adjacent to info.

void merge(inf * info) {
  if (info->next_free == (inf *)((char *)info + info->size)) {
    info->size += info->next_free->size;
    if (info->next_free->next_free != NULL) {
      info->next_free->next_free->prev_free = info;
    }
    info->next_free = info->next_free->next_free;
  }
}

void bf_free(void * ptr) {
  inf * info = (inf *)((char *)ptr - s_inf);
  // info->type = FREED;
  add_free_block(info);
  merge(info);
  if (info->prev_free != NULL) {
    merge(info->prev_free);
  }
}

unsigned int get_data_segment_size() {
  unsigned int ans = 0;
  void * heap_top = sbrk(0);
  ans = (unsigned long)heap_top - (unsigned long)first_block;
  return ans;
}

unsigned int get_data_segment_free_space_size() {
  unsigned int ans = 0;
  if (first_free == NULL) {
    return ans;
  }
  else {
    inf * it = first_free;
    while (it != NULL) {
      ans += (unsigned int)it->size;
      it = it->next_free;
    }
    return ans;
  }
  return ans;
}

void debug_print() {
  if (first_free == NULL) {
    printf("Nothing was freed.\n");
  }
  else {
    inf * it = first_free;
    while (it != NULL) {
      printf("Free block at %#lx, next free block at %#lx, size: %lx\n",
             (unsigned long)it,
             (unsigned long)it->next_free,
             it->size);
      it = it->next_free;
    }
  }

  printf("======================\n");
  return;
}

void debug_block(void * block) {
  inf * info = (inf *)((char *)block - s_inf);
  printf("Block at %#lx, size: %lx", (unsigned long)info, info->size);
  if (first_free == info || info->prev_free != NULL || info->next_free != NULL) {
    printf("block type: FREED\n");
  }
  else {
    printf("block type: ALLOCATED\n");
  }
  printf("======================\n");
  return;
}

void * ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  void * ptr = bf_malloc(size);
  pthread_mutex_unlock(&lock);
  return ptr;
}

void ts_free_lock(void * ptr) {
  pthread_mutex_lock(&lock);
  bf_free(ptr);
  pthread_mutex_unlock(&lock);
}

inf * nlexpand(size_t s) {
  size_t size = s + s_inf;
  pthread_mutex_lock(&lock);
  inf * info = (inf *)sbrk(size);
  pthread_mutex_unlock(&lock);
  if (info == (inf *)(-1)) {
    return NULL;
  }

  if (nlfirst_block == NULL) {
    nlfirst_block = info;
  }
  // last_block = info;
  info->size = size;
  // info->type = ALLOCATED;
  info->prev_free = NULL;
  info->next_free = NULL;
  return info;
}

// give the void * ptr represented by inf * info.

void * nlgive_ptr(inf * info) {
  void * ptr = (void *)((char *)info + s_inf);
  return ptr;
}

// split the free block from behind, return the allocated block.
// do not check if this block is large enough.
// input s should include s_inf.

inf * nlsplit(inf * info, size_t s) {
  info->size -= s;
  inf * new = (inf *)((char *)info + info->size);
  new->size = s;
  new->prev_free = NULL;
  new->next_free = NULL;
  // new->type = ALLOCATED;
  return new;
}

// mark entire free block as ALLOCATED, remove from link list.

void nloccupy(inf * info) {
  if (info->prev_free != NULL) {
    info->prev_free->next_free = info->next_free;
  }
  if (info->next_free != NULL) {
    info->next_free->prev_free = info->prev_free;
  }
  if (info == nlfirst_free) {
    nlfirst_free = info->next_free;
  }
  info->prev_free = NULL;
  info->next_free = NULL;
  // info->type = ALLOCATED;
}

// find free block with bf logic.

inf * nlbf_reuse(size_t s) {
  s += s_inf;
  inf * it = nlfirst_free;
  size_t b_size = ULONG_MAX;
  inf * b_inf = NULL;

  while (it != NULL) {
    if (it->size >= s && it->size < b_size) {
      b_size = it->size;
      b_inf = it;
    }
    if (b_size == s) {
      break;
    }
    it = it->next_free;
  }

  if (b_inf != NULL) {
    if (b_size - s <= s_inf) {
      nloccupy(b_inf);
      return b_inf;
    }
    else if (b_size - s > s_inf) {
      inf * new = nlsplit(b_inf, s);
      return new;
    }
  }
  return NULL;
}

void * nlbf_malloc(size_t size) {
  void * ptr;
  inf * target = nlbf_reuse(size);
  if (target == NULL) {
    target = nlexpand(size);
  }
  ptr = nlgive_ptr(target);
  return ptr;
}

// add info into the link list.

void nladd_free_block(inf * info) {
  if (nlfirst_free == NULL) {
    nlfirst_free = info;
    // last_free = info;
  }
  else {
    inf * it = nlfirst_free;

    if (it > info) {
      info->next_free = it;
      it->prev_free = info;
      nlfirst_free = info;
      return;
    }

    while (it != NULL) {
      if (it->next_free != NULL) {
        if (it->next_free > info) {
          it->next_free->prev_free = info;
          info->next_free = it->next_free;
          it->next_free = info;
          info->prev_free = it;
          return;
        }
      }
      else {
        it->next_free = info;
        info->prev_free = it;
        // last_free = it;

        return;
      }
      it = it->next_free;
    }
  }
  return;
}

// merge blocks adjacent to info.

void nlmerge(inf * info) {
  if (info->next_free == (inf *)((char *)info + info->size)) {
    info->size += info->next_free->size;
    if (info->next_free->next_free != NULL) {
      info->next_free->next_free->prev_free = info;
    }
    info->next_free = info->next_free->next_free;
  }
}

void nlbf_free(void * ptr) {
  inf * info = (inf *)((char *)ptr - s_inf);
  // info->type = FREED;
  nladd_free_block(info);
  nlmerge(info);
  if (info->prev_free != NULL) {
    nlmerge(info->prev_free);
  }
}

void * ts_malloc_nolock(size_t size) {
  return nlbf_malloc(size);
}
void ts_free_nolock(void * ptr) {
  nlbf_free(ptr);
}
