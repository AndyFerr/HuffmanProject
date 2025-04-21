#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>      // Para fopen, fread, fwrite, printf, etc
#include <stdlib.h>     // Para malloc, free, realloc
#include <string.h>     // Para strcmp, strcpy, strlen, etc
#include <stdint.h>     // Para tipos como uint8_t (útil pra bits)
#include <stdbool.h>    // Para usar `bool`, `true`, `false`



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

    // Completa com zeros os bits que faltam (à direita)
    buffer->byte <<= (8 - buffer->bits_usados);

    fwrite(&buffer->byte, 1, 1, f);

    // Zera o buffer
    buffer->byte = 0;
    buffer->bits_usados = 0;
}



void compactor(FILE *entrada, FILE *saida, BitBuffer *buffer) {

    while (fread(&c, 1, 1, entrada) == 1) {
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
