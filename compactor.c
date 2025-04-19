#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

#define BUFFER_SIZE 100


int main() {
    
    printf("\nInsert the name of the file:\n");
    
    char filename[BUFFER_SIZE];
    scanf("%s", filename);

    char new_file_name[BUFFER_SIZE];

    sprintf(new_file_name, "%s.huff", filename);
    
    FILE *original_file_ = fopen(filename, "rb");
    FILE *new_file = fopen(new_file_name, "wb");
    

    unsigned char c;
    BitBuffer buffer;
    
    compactor(original_file, new_file, &buffer);
    
    return 0;
}