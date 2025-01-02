#include "./hw4-library/memlib.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "./hw4-library/mm.h"

struct memory_region{
  size_t * start;
  size_t * end;
};

struct memory_region global_mem;
struct memory_region stack_mem;

void walk_region_and_mark(void* start, void* end);

// PROVIDED BY US (DO NOT CHANGE)
// ------------------------------
// grabbing the address and size of the global memory region from proc 
void init_global_range(){
  int next = 0;
  char file[100];
  char * line=NULL;
  size_t n=0;
  size_t read_bytes=0;
  size_t start, end;


  sprintf(file, "/proc/%d/maps", getpid());
  FILE * mapfile  = fopen(file, "r");
  if (mapfile==NULL){
    perror("opening maps file failed\n");
    exit(-1);
  }

  int counter=0;
  while ((read_bytes = getline(&line, &n, mapfile)) != -1) {
    // .data of hw4 executable
    if (strstr(line, "hw4") != NULL && strstr(line,"rw-p")){
      sscanf(line, "%lx-%lx", &start, &end);
      global_mem.start = (void*) start;
      global_mem.end = (void*) end;
      break;
    }

  }
  fclose(mapfile);
}

// marking related operations

int is_marked(unsigned int * chunk) {
  return ((*chunk) & 0x2) > 0;
}

void mark(unsigned int * chunk) {
  (*chunk)|=0x2;
}

void clear_mark(unsigned int * chunk) {
  (*chunk)&=(~0x2);
}

// chunk related operations

#define chunk_size(c)  ((*((unsigned int *)c))& ~(unsigned int)7 )
void* next_chunk(void* c) {
  if(chunk_size(c) == 0) {
    fprintf(stderr,"Panic, chunk is of zero size.\n");
  }
  if((c+chunk_size(c)) < mem_heap_hi())
    return ((void*)c+chunk_size(c));
  else
    return 0;
}

int in_use(void *c) {
  return *(unsigned int *)(c) & 0x1;
}

// FOR YOU TO IMPLEMENT
// --------------------
// the actual collection code
void sweep() {
  void * chunk = mem_heap_lo();
  while ((chunk_size(chunk) > 0) && (chunk < mem_heap_hi())) {
    if (is_marked(chunk)) {
      clear_mark(chunk);
    } else {
      mm_free(chunk + 4);
    }
    chunk = next_chunk(chunk);
  }
}

// determine if an 8-byte-aligned 8-byte value actually points to an
// in use block in the heap. if so, return a pointer to its header
// 
// Only called on values that are reachable, though it would work (in isolation) either way.
void * is_pointer(void * ptr) {
  // Here's a helpful print statement that shows some available memory operations and working with pointers
  printf("checking for pointeryness of %p between %p and %p\n",ptr, mem_heap_lo(), mem_heap_hi());
  if ((ptr >= mem_heap_lo()) && (ptr <= mem_heap_hi())) {
    void* chunk = mem_heap_lo();
    while ((chunk < mem_heap_hi()) && (chunk_size(chunk) > 0)) {
      if ((ptr >= chunk) && (ptr <= chunk + chunk_size(chunk))) {
        if (in_use(chunk))
          return chunk;
        else
          return NULL;
      }
      chunk = next_chunk(chunk);
    }
    // Q: how do I know if the pointer is pointing to an actual in-use block? A:
    // The in_use function tells you it *may* be pointing to an in-use block.
    // Q: in what way does ptr "look" like a pointer before this code is
    // called? A: It's just any 8-byte-aligned 8-byte-value. It doesn't particularly look like a pointer.
  }
  return NULL; // Return null if it is not a pointer 

}

// walk through memory specified by function arguments and mark
// each chunk
void walk_region_and_mark(void* start, void* end) {
  // TODO: Use char*, not void*.
  //fprintf(stderr, "walking %p - %p\n", start, end);
  // This function, on first call, gets the start and end of the stack or global variable region. 
  // The start is a lower value than the stack. It needs to find all the pointers between those addresses
  // (using is_pointer to determine if a given 8-byte thing is a pointer - are they all aligned to 8 bytes? A: Yes)
  for (void* potential_ptr_ptr = start; potential_ptr_ptr < end - 8; potential_ptr_ptr += 8) { // TODO: Test whether the -8 is needed.
    void* chunk;
    if ((chunk = is_pointer(*((void**) potential_ptr_ptr))) != NULL) { // If the potential_ptr_ptr points inside an in-use chunk on the heap... TODO: Remove the following part of the comment. Thanks Rory and Malek! This way (void**) I'm still dealing with a pointer.
      if (is_marked(chunk)) {
        continue;
      }
      mark(chunk); // mark the chunk that pointer is inside,
      walk_region_and_mark(chunk + 4, next_chunk(chunk)); // and walk that chunk, marking any pointers to the heap.
    }
  }
}

// PROVIDED BY US (DO NOT CHANGE)
// ------------------------------
// standard initialization 

void init_gc() {
  size_t stack_var;
  init_global_range();
  // since the stack grows down, the end is found first
  stack_mem.end=(size_t *)&stack_var;
}

void gc() {
  size_t stack_var;
  // grows down, so we set the start (which must be a lower value than the end) here,
  // after setting the end
  stack_mem.start=(size_t *)&stack_var;
  // walk memory regions
  walk_region_and_mark(global_mem.start,global_mem.end);
  walk_region_and_mark(stack_mem.start,stack_mem.end);
  sweep();
}
