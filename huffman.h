#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>      // To fopen, fread, fwrite, printf, etc
#include <stdlib.h>     // To malloc, free, realloc
#include <string.h>     // To strcmp, strcpy, strlen, etc
#include <stdint.h>     // To tipos como uint8_t (útil pra bits)
#include <stdbool.h>    // To `bool`, `true`, `false`
#include "pqueue_heap.h"



/*
    FUNCTIONS TO COMPACT THE FILE.
*/

// BIT BUFFER TO STORE THE BITS UNTIL A BYTE IS FULL AND WRITE IT
typedef struct {
    unsigned char byte;  // Armazena os bits temporariamente
    int bits_usados;     // De 0 a 7
} BitBuffer;


// FUNTION TO ADD A BIT TO THE BUFFER
void bit_buffer_add(BitBuffer *buffer, int bit) {
    buffer->byte <<= 1;             // move the bit to the left
    buffer->byte |= (bit & 1);      // move the bit to the right
    buffer->bits_usados++;
}


void write_buffer(FILE *f, BitBuffer *buffer) {
    if (buffer->bits_usados == 0) return;

    // Complete then missing bits at the right of the byte
    buffer->byte <<= (8 - buffer->bits_usados);

    fwrite(&buffer->byte, 1, 1, f);

    // clean the buffer
    buffer->byte = 0;
    buffer->bits_usados = 0;
}


PRIORITY_QUEUE* create_huffman_table(FILE *enter_file) {
    int freq[256] = {0}; // Frequency of each character
    unsigned char c;

    // 1. Count the frequency of each character
    while (fread(&c, 1, 1, enter_file) == 1) {
        freq[c]++;
    }

    // 2. Creates the priority queue
    PRIORITY_QUEUE* pq = create_queue();

    // 3. Insert in the PQ only the values > 0
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            insert(pq, i, freq[i]);
        }
    }

    return pq;
}


void compactor(FILE *enter_file, FILE *saida, BitBuffer *buffer) {

    while (fread(&c, 1, 1, enter_file) == 1) {
        char *codigo = tabela_huffman[c];

        for (int i = 0; codigo[i] != '\0'; i++) {
            adiciona_bit_ao_buffer(&buffer, codigo[i] == '1');

            if (buffer->bits_usados == 8) {
                write_buffer(saida, buffer);
            }
        }
    }

    // Write the rest of the buffer if there's something lower than 8
    write_buffer(saida, buffer);
}


#endif // HUFFMAN_H
