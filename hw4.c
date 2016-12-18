//Ankita Tank
//Garbage Collector
//HW4
//CS361
//atank2

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct memory_region{
  size_t * start;
  size_t * end;
};

struct memory_region global_mem;
struct memory_region heap_mem;
struct memory_region stack_mem;

void walk_region_and_mark(void* start, void* end);

//how many ptrs into the heap we have
#define INDEX_SIZE 1000
void* heapindex[INDEX_SIZE];


//grabbing the address and size of the global memory region from proc 
void init_global_range(){
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
    if (strstr(line, "hw4")!=NULL){
      ++counter;
      if (counter==3){
        sscanf(line, "%lx-%lx", &start, &end);
        global_mem.start=(size_t*)start;
        // with a regular address space, our globals spill over into the heap
        global_mem.end=malloc(256);
        free(global_mem.end);
      }
    }
    else if (read_bytes > 0 && counter==3) {
      if(strstr(line,"heap")==NULL) {
        // with a randomized address space, our globals spill over into an unnamed segment directly following the globals
        sscanf(line, "%lx-%lx", &start, &end);
        printf("found an extra segment, ending at %zx\n",end);						
        global_mem.end=(size_t*)end;
      }
      break;
    }
  }
  fclose(mapfile);
}


//marking related operations

int is_marked(size_t* chunk) {
  return ((*chunk) & 0x2) > 0;
}

void mark(size_t* chunk) {
  (*chunk)|=0x2;
}

void clear_mark(size_t* chunk) {
  (*chunk)&=(~0x2);
}

// chunk related operations

#define chunk_size(c)  ((*((size_t*)c))& ~(size_t)3 ) 
void* next_chunk(void* c) { 
  if(chunk_size(c) == 0) {
    printf("Panic, chunk is of zero size.\n");
  }
  if((c+chunk_size(c)) < sbrk(0))
    return ((void*)c+chunk_size(c));
  else 
    return 0;
}
int in_use(void *c) { 
  return (next_chunk(c) && ((*(size_t*)next_chunk(c)) & 1));
}


// index related operations

#define IND_INTERVAL ((sbrk(0) - (void*)(heap_mem.start - 1)) / INDEX_SIZE)
void build_heap_index() {
  // TODO
}




//The Function Sweep - This function free's the chunk in the block assigned to it andIt should be marked.
//Free's the chunk not in use as it is iterating through the heap using walk_region fucntion.
//reference from book pg no. 868 
//function void sweep()

void sweep() 
{
	//printf("in function sweep ");
  size_t* temp_ptr;
  int i=0;					//temperary pointer for to iterate through the heap
  size_t* chunk_head = (void *)(heap_mem.start - 1);   // pointer to the head of the heap
  size_t* chunk_end = heap_mem.end;			// pointer to the end of the heap 

  while(chunk_head  && chunk_head < (size_t *)sbrk(0))   // do the loop until there are no more head's in the heap
  { 
//	printf("the while ");
    temp_ptr = (size_t *) next_chunk(chunk_head);      // temperary pointer pointing to the next chunk (chunck after head)


//check for the marked chunk

    if(   in_use(chunk_head)  && !is_marked(chunk_head)) 
    {
	//	printf("it is cleared");
	//clear_mark(chunk_head);		//clear the chunk if it is marked
    
//		chunk_head=temp_ptr;
	free(chunk_head +1);
	//chunk_end=chunk_end-1;	
	} 
	else if( is_marked (chunk_head)  ){

	clear_mark(chunk_head); 
 	//free( chunk_head  + 1);
    }
	i++;
	//printf("now my i is %d ", i);
	//printf("one loop completed");
     chunk_head = temp_ptr;
  }

}





//function - is_pointer -> this function returns the pointers which are being used in the heap.


size_t * is_pointer(size_t * ptr) 
{
  size_t *chunk_head = heap_mem.start -1;
  size_t *chunk_last = heap_mem.end;
 
 //if there are no chunk found in the heap return 0
  if(ptr < heap_mem.start || ptr >= heap_mem.end) 
  {
    //return NULL;
	return 0;
  } 

  //while there is a chunk in the heap and the chunk.next!=0
 
   while(chunk_head && chunk_head < chunk_last )
   {
   //temp_ptr will hold the sub size of that chunk (one at a time)

    int temp_ptr = chunk_size(chunk_head)/sizeof(size_t); 

	// if that chunk is use and head+that size > the ptr which is pointing that specific chunck and teh head is < the ptr 
    if(    in_use(chunk_head)   	&&    chunk_head+temp_ptr > ptr		&&  chunk_head <= ptr)
    { 
	//printf("my ptr is pointing to %zx  " , ptr);
	//return chuck_head+1;
	return chunk_head; 
    
    }
	// chunk_head = (size_t*)next_chunk(chunk_last+1);
    chunk_head = (size_t*)next_chunk(chunk_head); 
  }
    return NULL; 
}




//function walk_region_and_mark -> this function marks all the pointers in use from the heap 

void walk_region_and_mark(void* start, void* end) 
{
  //printf("here inwalk");
size_t * temp_ptr =  start; 
int counter_marked=0;
//if(start==NULL)
	//print("null");

  while( temp_ptr < (size_t *)end ) 
  {
       
	//make a chunk variable to go through each chunk in heap and check if that specific is marked 
	size_t * chunk_exists = is_pointer((size_t *)*temp_ptr); 

	//there is a chunk which is not NULL and if it is not marked, then mark it
       if(chunk_exists  && !is_marked(chunk_exists)) 
        { 
	//printf("inwalk e);
	//mark the chunk is it is not marked
	mark(chunk_exists);

	//recursively walk through the chunk and "walk through" each and mark if it notmarked;  
	 walk_region_and_mark(chunk_exists, ((void*)chunk_exists)+chunk_size(chunk_exists));
 	//walk_region_and_mark(chunk_exists, ((void*)chunk_exists));
	counter_marked++;		 
}

//     printf("the unmarked marked ones are %d", countermarked);
    temp_ptr++; 
  }
}




// standard initialization 

void init_gc() {
  size_t stack_var;
  init_global_range();
  heap_mem.start=malloc(512);
  //since the heap grows down, the end is found first
  stack_mem.end=(size_t *)&stack_var;
}

void gc() {
  size_t stack_var;
  heap_mem.end=sbrk(0);
  //grows down, so start is a lower address
  stack_mem.start=(size_t *)&stack_var;

  // build the index that makes determining valid ptrs easier
  // implementing this smart function for collecting garbage can get bonus;
  // if you can't figure it out, just comment out this function.
  // walk_region_and_mark and sweep are enough for this project.
  build_heap_index();

  //walk memory regions
  walk_region_and_mark(global_mem.start,global_mem.end);
  walk_region_and_mark(stack_mem.start,stack_mem.end);
  sweep();
}



