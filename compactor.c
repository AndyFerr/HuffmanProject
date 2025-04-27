#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "pqueue_heap.h"

#define BUFFER_SIZE 100


int main() {
    
    printf("\nInsert the name of the file:\n");
    
    char filename[BUFFER_SIZE];
    scanf("%s", filename);

    while(1){
        FILE *original_file_ = fopen(filename, "rb");
        if (original_file == NULL) {
            perror("Error opening file");
            exit(1);
        }else{
            break;
        }
    }
    
    char new_file_name[BUFFER_SIZE];
    sprintf(new_file_name, "%s.huff", filename);

    // Creates the new file
    FILE *new_file = fopen(new_file_name, "wb");
    

    PRIORITY_QUEUE* huff_queue = create_queue(original_file);
    NODE* root = build_huffman_tree(huff_queue);

    char* huff_table[256] = {0};
    char path[256];
    create_huffman_table(root, path, 0, huff_table);
    
    unsigned char c;
    BitBuffer buffer;
    
    compactor(original_file, new_file, &buffer);
    
    return 0;
}