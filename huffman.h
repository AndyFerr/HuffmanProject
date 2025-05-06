#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pqueue_heap.h"

// Cria a fila de prioridade com frequência de cada caractere no arquivo
void create_huff_queue(FILE *input_file, PRIORITY_QUEUE** pq1, PRIORITY_QUEUE** pq2) {
    int freq[256] = {0};
    unsigned char c;

    // Count character frequencies
    while (fread(&c, 1, 1, input_file) == 1) {
        freq[c]++;
    }

    // Create nodes for characters with non-zero frequency and insert them into both queues
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            NODE* node = create_node(i, freq[i], NULL, NULL);
            insert(*pq1, node);
            insert(*pq2, node);
        }
    }
}

// Constrói a árvore de Huffman a partir da fila de prioridade
NODE* build_huffman_tree(PRIORITY_QUEUE* pq) {
    while (pq->size > 1) {
        NODE* left = remove_lower(pq);
        NODE* right = remove_lower(pq);

        // Create a parent node with the left and right children
        NODE* parent = create_node('\0', left->frequency + right->frequency, left, right);

        // Insert the parent node back into the priority queue
        insert(pq, parent);
    }

    // The last remaining node is the root of the Huffman tree
    return remove_lower(pq);
}

// Gera a tabela de Huffman com os códigos binários dos caracteres
typedef struct {
    uint32_t code;
    int length;
} HuffmanCode;

void create_huffman_table(NODE* root, uint32_t code, int depth, HuffmanCode huff_table[256]) {
    if (!root) return;

    if (!root->left && !root->right) {
        huff_table[root->character].code = code;
        huff_table[root->character].length = depth;
        return;
    }

    create_huffman_table(root->left, (code << 1), depth + 1, huff_table);
    create_huffman_table(root->right, (code << 1) | 1, depth + 1, huff_table);
}

// Calcula quantos bits totais serão escritos no corpo compactado
int calculate_bits_trashed(PRIORITY_QUEUE* pq, HuffmanCode huff_table[256]) {
    int bit_amount = 0;

    while (pq->size > 0) {
        NODE* node = remove_lower(pq);
        if (node && huff_table[node->character].length > 0) {
            bit_amount += node->frequency * huff_table[node->character].length;
        }
    }

    return bit_amount;
}

// Conta o número de nós da árvore (para o tamanho da árvore no cabeçalho)
int count_tree_size(NODE* root) {
    if (!root) return 0;
    return count_tree_size(root->left) + count_tree_size(root->right) + 1;
}

int is_leaf(NODE* node) {
    return node && !node->left && !node->right;
}

// Escreve a árvore de Huffman no arquivo de saída
void write_tree(NODE* root, FILE* output_file) {
    if (root) {
        if (is_leaf(root)) {
            fputc('*', output_file);
            if (root->character == '*' || root->character == '\\') {
                fputc('\\', output_file);
            }
            fputc(root->character, output_file);
        } else {
            fputc('*', output_file);
        }
        write_tree(root->left, output_file);
        write_tree(root->right, output_file);
    }
}

// Escreve o cabeçalho no novo arquivo (lixo, tamanho da árvore, árvore)
void write_header(PRIORITY_QUEUE* pq, HuffmanCode huff_table[256], FILE *output_file, NODE* root) {

    int total_bits = calculate_bits_trashed(pq, huff_table);
    int trash = ((8 - (total_bits % 8)) % 8);
    int tree_size = count_tree_size(root);

    unsigned short header = (trash << 13) | tree_size;
    unsigned char byte1 = header >> 8;
    unsigned char byte2 = header & 0xFF;

    fwrite(&byte1, 1, 1, output_file);
    fwrite(&byte2, 1, 1, output_file);

    write_tree(root, output_file);
}

// Estrutura para armazenar bits até completar um byte
typedef struct {
    unsigned char byte;
    int bits_used;
} BitBuffer;

void bit_buffer_add(BitBuffer *bit_buffer, int bit) {
    bit_buffer->byte <<= 1;
    bit_buffer->byte |= (bit & 1);
    bit_buffer->bits_used++;
}

void write_buffer(FILE *f, BitBuffer *bit_buffer) {
    if (bit_buffer->bits_used == 0) return;

    unsigned char temp_byte = bit_buffer->byte << (8 - bit_buffer->bits_used);
    fwrite(&temp_byte, 1, 1, f);

    bit_buffer->byte = 0;
    bit_buffer->bits_used = 0;
}

// Escreve os dados compactados no novo arquivo
void compactor(FILE *input_file, FILE *output_file, HuffmanCode huff_table[256]) {
    unsigned char c;
    BitBuffer bit_buffer = {0, 0}; // Initialize the bit buffer

    // Read each character from the input file
    while (fread(&c, 1, 1, input_file) == 1) {
        HuffmanCode code = huff_table[c];

        // Skip characters with no Huffman code
        //if (code.length == 0) continue;

        // Add each bit of the Huffman code to the bit buffer
        for (int i = code.length - 1; i >= 0; i--) {
            bit_buffer_add(&bit_buffer, (code.code >> i) & 1);

            // Write the buffer to the file when it is full
            if (bit_buffer.bits_used == 8) {
                write_buffer(output_file, &bit_buffer);
            }
        }
    }

    // Write any remaining bits in the buffer
    write_buffer(output_file, &bit_buffer);
}

void free_huffman_tree(NODE* root) {
    if (root == NULL) return;

    free_huffman_tree(root->left);
    free_huffman_tree(root->right);
    free(root);
}



// Imprime a tabela de Huffman
void print_huff_table(HuffmanCode huff_table[256]) {
    for (int i = 0; i < 256; i++) {
        if (huff_table[i].length > 0) {
            printf("'%c' (%d): Code: ", i, i);
            for (int j = huff_table[i].length - 1; j >= 0; j--) {
                printf("%d", (huff_table[i].code >> j) & 1);
            }
            printf(", Length: %d\n", huff_table[i].length);
        }
    }
}

void print_huffman_tree(NODE* root, int level) {
    if (!root) return;

    print_huffman_tree(root->right, level + 1);

    for (int i = 0; i < level; i++) {
        printf("    ");
    }

    if (root->left == NULL && root->right == NULL) {
        // Nó folha: imprime caractere e frequência
        if (root->character >= 32 && root->character <= 126) {
            // Imprime caractere visível
            printf("'%c' (%d)\n", root->character, root->frequency);
        } else {
            // Imprime valor numérico do caractere
            printf("0x%02X (%d)\n", root->character, root->frequency);
        }
    } else {
        // Nó interno: só imprime frequência
        printf("* (%d)\n", root->frequency);
    }

    print_huffman_tree(root->left, level + 1);
}


#endif // HUFFMAN_H
