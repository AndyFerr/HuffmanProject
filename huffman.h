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



/*
    Creates the priority queue for each character
*/
void create_huff_queue(FILE *input_file, PRIORITY_QUEUE** pq1, PRIORITY_QUEUE** pq2) {
    int freq[256] = {0}; // Frequency of each character
    unsigned char c;

    // 1. Count the frequency of each character
    while (fread(&c, 1, 1, input_file) == 1) {
        freq[c]++;
    }

    // 2. Insert in the PQ only the values > 0
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            insert(*pq1, i, freq[i]);
            insert(*pq2, i, freq[i]);
        }
    }
}

/*
    Creates the huffman tree
*/
NODE* build_huffman_tree(PRIORITY_QUEUE* pq) {
    while (pq->size > 1) {
        NODE* left = remove_lower(pq);
        NODE* right = remove_lower(pq);
        NODE* parent = create_node('\0', left->frequency + right->frequency, left, right);
        insert(pq, parent->character, parent->frequency);
    }
    return remove_lower(pq); // return the roof tree
}


void create_huffman_table(NODE* root, char* path, int depth, char* huff_table[256]) {
    if (root == NULL) return;

    if (root->left == NULL && root->right == NULL) {
        path[depth] = '\0'; // Finalize the string
        huff_table[root->character] = strdup(path); // Save the path in the table in the respective index
        return;
    }

    // Goes to the left and adds 0 to thr path
    path[depth] = '0';
    create_huffman_table(root->left, path, depth + 1, huff_table);

    // Goes to the right and adds 1 to thr path
    path[depth] = '1';
    create_huffman_table(root->right, path, depth + 1, huff_table);
}


/*
    Calculate the amount of bits trashed at the end of the file by the huffman tree
*/
int calculate_bits_trashed(PRIORITY_QUEUE* pq, char* huff_table[256]) {
    int bit_amount = 0;
    while (pq->size > 0) {
        NODE* left = remove_lower(pq);
        if (left) {
            bit_amount += left->frequency * strlen(huff_table[left->character]);
        }

        NODE* right = remove_lower(pq);
        if (right) {
            bit_amount += right->frequency * strlen(huff_table[right->character]);
        }
    }
    return bit_amount;
}

int count_tree_size(NODE* root) {
    if (root == NULL) return 0;
    return count_tree_size(root->left) + count_tree_size(root->right) + 1;
}

int is_leaf(NODE* node) {
    return node->left == NULL && node->right == NULL;
}

void write_header(PRIORITY_QUEUE* pq, char* huff_table[256], FILE *output_file, NODE* root) {
    
    /*
        Calculate the amounts of trash bits at the end of the file
        according to the huffman table
    */
    int amount_of_bits = calculate_bits_trashed(pq, huff_table);
    int trash = ((8 - (amount_of_bits % 8)) % 8);


    // Calculates the size of the huffman tree
    int tree_size = count_tree_size(root);


    // Creates the header
    unsigned short header = (trash << 13) | tree_size;

    // Separates the two bytes
    unsigned char byte1 = header >> 8;    // higher bits
    unsigned char byte2 = header & 0xFF;  // lower bits

    // Writes the header
    fwrite(&byte1, 1, 1, output_file);
    fwrite(&byte2, 1, 1, output_file);

    /*
        Writes the huffman tree in the required format
    */
    void write_tree(NODE* root, FILE* output_file) {
        if (root != NULL) {
            if (is_leaf(root)) {
                fputc('*', output_file);
                if (root->character == '*' || root->character == '\\') {
                    fputc('\\', output_file); // escape
                }
                fputc(root->character, output_file);
            } else {
                fputc('*', output_file);
            }
            write_tree(root->left, output_file);
            write_tree(root->right, output_file);
        }
    }
}





// BIT BUFFER TO STORE THE BITS UNTIL A BYTE IS FULL AND WRITE IT
typedef struct {
    unsigned char byte;  // Stores 8 bits temporarilya
    int bits_used;     // 0 to 7
} BitBuffer;


// FUNTION TO ADD A BIT TO THE BUFFER
void bit_buffer_add(BitBuffer *buffer, int bit) {
    buffer->byte <<= 1;             // move the bit to the left
    buffer->byte |= (bit & 1);      // move the bit to the right
    buffer->bits_used++;
}

void write_buffer(FILE *f, BitBuffer *buffer) {
    if (buffer->bits_used == 0) return;

    // Complete then missing bits at the right of the byte
    buffer->byte <<= (8 - buffer->bits_used);

    fwrite(&buffer->byte, 1, 1, f);

    // clean the buffer
    buffer->byte = 0;
    buffer->bits_used = 0;
}

void compactor(FILE *input_file, FILE *output_file, char* huff_table[256]) {

    unsigned char c;
    BitBuffer buffer = {0, 0};
    
    while (fread(&c, 1, 1, input_file) == 1) {
        char *codigo = huff_table[c];
            
        for (int i = 0; codigo[i] != '\0'; i++) {
            bit_buffer_add(&buffer, codigo[i] == '1');

            if (buffer.bits_used == 8) {
                write_buffer(output_file, &buffer);
            }
        }
    }

    // Write the rest of the buffer if there's something lower than 8
    write_buffer(output_file, &buffer);
}


#endif // HUFFMAN_H






