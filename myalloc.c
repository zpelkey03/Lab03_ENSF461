//Our myalloc file for Lab 6
#include <stddef.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>

#include "myalloc.h"

// Define MAP_ANONYMOUS if not defined
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif

int statusno;  // Declare the statusno variable.

static void *arena_start = NULL;

int myinit(size_t size) {
    //check for invalid size
    if(size > MAX_ARENA_SIZE || size == 0){
        statusno = ERR_BAD_ARGUMENTS;
        return ERR_BAD_ARGUMENTS;
    }

    // Get the system's page size
    size_t page_size = sysconf(_SC_PAGE_SIZE);

    // Adjust size with page boundaries (e.g., 4096 bytes)
    size_t adjusted_size = (size + page_size - 1) & ~(page_size - 1);


    printf("Initializing arena:\n");
    printf("...requested size %zu bytes\n", size);
    printf("...pagesize is %zu bytes\n", page_size);
    printf("...adjusting size with page boundaries\n");
    printf("...adjusted size is %zu bytes\n", adjusted_size);

    arena_start = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if (arena_start == MAP_FAILED){
        statusno = ERR_SYSCALL_FAILED;
        return ERR_SYSCALL_FAILED;
    }

    printf("...mapping arena with mmap()\n");
    printf("...arena starts at %p\n", arena_start);
    printf("...arena ends at %p\n", arena_start + adjusted_size);

    statusno = 0; //Success

    // Create a single large chunk that covers the entire arena
    node_t* initial_chunk = (node_t*)arena_start;
    initial_chunk->size = size;
    initial_chunk->is_free = 1;
    initial_chunk->fwd = NULL;
    initial_chunk->bwd = NULL;
    
    return adjusted_size;
}

int mydestroy(){
    if (arena_start == NULL){
        statusno = ERR_UNINITIALIZED;
        return ERR_UNINITIALIZED;
    }

    //Reset state variables
    arena_start = NULL;
    statusno = 0;

    printf("Destroying Arena:\n");
    printf("...unmapping arena with munmap()\n");

    return 0;
}

void* myalloc(size_t size) {
    if (size <= 0 || arena_start == NULL) {
        statusno = ERR_UNINITIALIZED;
        return NULL;
    }

    
    // Search for a free chunk within the arena (your allocation logic goes here)
    node_t* current = (node_t*)arena_start;

    while (current != NULL) {
        // Calculate the total size needed (including the header)
        size_t total_size = size ;
        

        if(current->bwd != NULL){
            total_size -= sizeof(node_t);
        }

        if (current->is_free && current->size >= total_size && total_size < getpagesize()) {
            // Found a suitable free chunk
            size_t remaining_size = current->size - total_size - sizeof(node_t);
            // Split the chunk if there's enough space for a new free chunk
           
            if (remaining_size > sizeof(node_t)) {
                // Create a new free chunk
                node_t* new_chunk = (node_t*)((char*)current + total_size + sizeof(node_t));
                new_chunk->size = remaining_size - sizeof(node_t);
                new_chunk->is_free = 1;
                new_chunk->fwd = current->fwd;
                new_chunk->bwd = current;
                current->size = size;
                current->is_free = 0;
                current->fwd = new_chunk;

                     
                // Return a pointer to the application area (after the header)
                return (void*)((char*)current + sizeof(node_t));
            } else {
                // Use the entire chunk if there's no space for a new free chunk
                current->is_free = 0;
                if(current->size == getpagesize()){
                    current->size -= sizeof(node_t);
                }
                // Return a pointer to the application area (after the header)
                return (void*)((char*)current + sizeof(node_t));
            }
        }

        current = current->fwd;
    }
    statusno = ERR_OUT_OF_MEMORY;
    return NULL; // No suitable free chunk found
}



void myfree(void* ptr) {
    if (ptr == NULL || arena_start == NULL) {
        return; // Invalid input
    }

    // Find the header of the chunk using the provided calculation
    node_t* header = (node_t*)((char*)ptr - sizeof(node_t));

    // Mark the chunk as free
    header->is_free = 1;
    
    if(header->fwd == NULL && header->bwd == NULL){
        
    }
    // Perform coalescing (merging with adjacent free chunks)
    node_t* current = (node_t*)arena_start;
    while (current) {
        if (current->is_free) {
            // Check if the next chunk is also free and merge them
            node_t* next = current->fwd;
            while (next && next->is_free ) {
                current->size += next->size + sizeof(node_t); // Merge sizes
                current->fwd = next->fwd;     // Update the forward pointer
                next = current->fwd;         // Move to the next potential free chunk
            }
        }
        current = current->fwd;
    }
}
