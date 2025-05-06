#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "pqueue_heap.h"

#define BUFFER_SIZE 1000

int main(){

    char filename[BUFFER_SIZE];
    scanf("%s", filename);

    FILE* original_file = fopen(filename, "rb");
    if (original_file == NULL) {
        perror("Error opening file");
        return 1;
    }

    PRIORITY_QUEUE* huff_queue1 = create_queue();
    PRIORITY_QUEUE* huff_queue2 = create_queue();

    create_huff_queue(original_file, &huff_queue1, &huff_queue2);

    fclose(original_file);

    print_priority_queue(huff_queue1);

    NODE* root = build_huffman_tree(huff_queue1);

    print_huffman_tree(root, 0);
    printf("\n\n");


    HuffmanCode huff_table[256] = {0}; // Initialize the Huffman table
    uint32_t code = 0; // Initialize code as an integer
    create_huffman_table(root, code, 0, huff_table); // Pass code as a pointer
    
    print_huff_table(huff_table);

    printf("\n\n");
    return 0;

}