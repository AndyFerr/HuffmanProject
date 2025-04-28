#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "pqueue_heap.h"

#define BUFFER_SIZE 1000


int main() {
    
    printf("\nInsert the name of the file:\n");
    
    char filename[BUFFER_SIZE];
    scanf("%s", filename);

    FILE* original_file = fopen(filename, "rb");

    while(1){
        if (original_file == NULL) {
            perror("Error opening file");
            exit(1);
        }else{
            break;
        }
    }
    
    char new_file_name[BUFFER_SIZE+5];
    snprintf(new_file_name, sizeof(new_file_name)+5, "%s.huff", filename);


    // Creates the new file
    FILE *new_file = fopen(new_file_name, "wb");
    
    /*
        Creates the priority queue for the huffman tree
        and a copy to count the amount of bits in the file
    */
    PRIORITY_QUEUE* huff_queue1 = create_queue();
    PRIORITY_QUEUE* huff_queue2 = create_queue();

    // Creates priority huffman queue and the huffman tree
    create_huff_queue(original_file, &huff_queue1, &huff_queue2);
    NODE* root = build_huffman_tree(huff_queue1);


    // Creates the huffman table
    char* huff_table[256] = {0};
    char path[256];
    create_huffman_table(root, path, 0, huff_table);


    /*
        define and writes the header on the new file
    */
    write_header(huff_queue2, huff_table, new_file, root);

    
    compactor(original_file, new_file, huff_table);
    
    return 0;
}




