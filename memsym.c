#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h> // Include this header for uint32_t

#define TRUE 1
#define FALSE 0

#define TLB_SIZE 8 // Assuming a TLB size of 8 entries

// Define the structure for a TLB entry
typedef struct {
    uint32_t vpn;
    uint32_t pfn;
    int valid; // Indicates whether the entry is valid (1 for valid, 0 for invalid)
    // Add any other necessary fields for TLB entries
} TLBEntry;

// Declare the TLB as a global variable
TLBEntry tlb[TLB_SIZE]; // Array of TLB entries


// Output file
FILE* output_file;

int current_pid;
int define_called = 0;

// Define two registers
uint32_t register0;
uint32_t register1;

uint32_t* physical_memory;

// Constants for memory parameters
uint32_t OFF; // Number of bits for offset
uint32_t PFN; // Number of bits for PFN
uint32_t VPN; // Number of bits for VPN

int current_index = 0;

// TLB replacement strategy (FIFO or LRU)
char* strategy;

struct PageTableEntry {
    int vpn;
    int pfn;
    struct PageTableEntry* next;
};

struct PageTableEntry* page_tables[4] = {NULL, NULL, NULL, NULL};

void initialize_simulator(uint32_t off, uint32_t pfn, uint32_t vpn) {
    if (define_called == 1) {
        fprintf(output_file, "Current PID: %d. Error: multiple calls to define in the same trace\n", current_pid);
        exit(1);
    }

    // Set the memory parameters
    OFF = off;
    PFN = pfn;
    VPN = vpn;

    // Initialize TLB
    for (int i = 0; i < 8; i++) {
        // Mark all TLB entries as invalid
        tlb[i].valid = 0;
    }

    // Initialize per-process page tables
    for (int i = 0; i < 4; i++) {
        // Initialize each process's page table with all entries as invalid
        for (int j = 0; j < (1 << VPN); j++) {
            page_tables[i][j].vpn = -1; // Initialize to an invalid value
            page_tables[i][j].pfn = -1;
            page_tables[i][j].next = NULL;
        }
    }

    // Calculate the length of the physical memory array
    uint32_t mem_size = 1 << (OFF + PFN);

    // Allocate memory for the physical memory as an array of uint32_t integers
    physical_memory = (uint32_t*)malloc(mem_size * sizeof(uint32_t));

    if (physical_memory == NULL) {
        fprintf(output_file, "Error: Failed to allocate memory for physical memory\n");
        exit(1);
    }

    // Initialize physical memory locations to 0
    for (uint32_t i = 0; i < mem_size; i++) {
        physical_memory[i] = 0;
    }

    // Additional initialization code for TLB and page tables

    // Flag to indicate 'define' has been called
    define_called = 1;
    current_pid = 0;
    // Print initialization message
    fprintf(output_file, "Current PID: %d. Memory instantiation complete. OFF bits: %u. PFN bits: %u. VPN bits: %u\n", current_pid, OFF, PFN, VPN);
}

void save_registers_to_physical_memory() {
    // Save the values of register0 and register1 to physical memory
    physical_memory[2*current_pid] = register0;
    physical_memory[2*current_pid+1] = register1;
}

void load_registers_from_physical_memory() {
    // Load the values of register0 and register1 from physical memory
    register0 = physical_memory[2*current_pid];
    register1 = physical_memory[2*current_pid+1];
}



void context_switch(uint32_t pid) {
    if (pid < 0 || pid > 3) {
        fprintf(output_file, "Current PID: %d. Invalid context switch to process %u\n", current_pid, pid);
        exit(1);
    }

    save_registers_to_physical_memory();
    
    // Switch the context to the new process
    current_pid = pid;

    load_registers_from_physical_memory();

    fprintf(output_file,"Current PID: %d. Switched execution context to process: %d\n", current_pid, current_pid);
}

void map(int vpn, int pfn) {
    struct PageTableEntry* new_entry = (struct PageTableEntry*)malloc(sizeof(struct PageTableEntry));
    new_entry->vpn = vpn;
    new_entry->pfn = pfn;
    new_entry->next = NULL;

    // Find the current process's page table
    struct PageTableEntry* page_table = page_tables[current_pid];

    if (page_table == NULL) {
        // This is the first entry in the page table
        page_tables[current_pid] = new_entry;
    } else {
        // Add the new entry to the end of the page table
        struct PageTableEntry* current_entry = page_table;
        while (current_entry->next != NULL) {
            current_entry = current_entry->next;
        }
        current_entry->next = new_entry;
    }

    // You can also update your TLB here if you are using one.
}

char** tokenize_input(char* input) {
    char** tokens = NULL;
    char* token = strtok(input, " ");
    int num_tokens = 0;

    while (token != NULL) {
        num_tokens++;
        tokens = realloc(tokens, num_tokens * sizeof(char*));
        tokens[num_tokens - 1] = malloc(strlen(token) + 1);
        strcpy(tokens[num_tokens - 1], token);
        token = strtok(NULL, " ");
    }

    num_tokens++;
    tokens = realloc(tokens, num_tokens * sizeof(char*));
    tokens[num_tokens - 1] = NULL;

    return tokens;
}

int main(int argc, char* argv[]) {
    const char usage[] = "Usage: memsym.out <strategy> <input trace> <output trace>\n";
    char* input_trace;
    char* output_trace;
    char buffer[1024];

    // Parse command line arguments
    if (argc != 4) {
        printf("%s", usage);
        return 1;
    }
    strategy = argv[1];
    input_trace = argv[2];
    output_trace = argv[3];

    // Open input and output files
    FILE* input_file = fopen(input_trace, "r");
    output_file = fopen(output_trace, "w");  

    while ( !feof(input_file) ) {
        // Read input file line by line
        char *rez = fgets(buffer, sizeof(buffer), input_file);
        if ( !rez ) {
            fprintf(stderr, "Reached end of trace. Exiting...\n");
            return -1;
        } else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }
        char** tokens = tokenize_input(buffer);

        // TODO: Implement your memory simulator
        // Check if the first token is "define" and process accordingly
        if (strcmp(tokens[0], "define") == 0) {
            
            // Ensure that the 'define' instruction has the required number of arguments
            if (tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL) {
                fprintf(output_file, "Error: Invalid 'define' instruction\n");
                return -1;
            }

            uint32_t off = atoi(tokens[1]);
            uint32_t pfn = atoi(tokens[2]);
            uint32_t vpn = atoi(tokens[3]);

            // Initialize the simulator with the provided parameters
            initialize_simulator(off, pfn, vpn);

            define_called = 1;
        }

        // Check for the "ctxswitch" instruction and process accordingly
        if (strcmp(tokens[0], "ctxswitch") == 0) {
            uint32_t pid = atoi(tokens[1]);
            context_switch(pid);
        }

        if (strcmp(tokens[0], "map") == 0) {
            // Ensure that 'define' has been called before map
            if (define_called == 0) {
                fprintf(stderr, "Error: attempt to execute instruction before define\n");
                return -1;
            }

            // Ensure that the 'map' instruction has the required number of arguments
            if (tokens[1] == NULL || tokens[2] == NULL) {
                fprintf(stderr, "Error: Invalid 'map' instruction\n");
                return -1;
            }

            uint32_t vpn = atoi(tokens[1]);
            uint32_t pfn = atoi(tokens[2]);

            // Call the map function to handle the mapping
            map(vpn, pfn);
        }

        // Deallocate tokens
        for (int i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
    }

    // Close input and output files
    fclose(input_file);
    fclose(output_file);

    return 0;
}
