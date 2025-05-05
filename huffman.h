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

    while (fread(&c, 1, 1, input_file) == 1) {
        freq[c]++;
    }

    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            insert(*pq1, i, freq[i]);
            insert(*pq2, i, freq[i]);
        }
    }
}

// Constrói a árvore de Huffman a partir da fila de prioridade
NODE* build_huffman_tree(PRIORITY_QUEUE* pq) {
    while (pq->size > 1) {
        NODE* left = remove_lower(pq);
        NODE* right = remove_lower(pq);
        NODE* parent = create_node('\0', left->frequency + right->frequency, left, right);
        insert(pq, parent->character, parent->frequency);
    }
    return remove_lower(pq);
}

// Gera a tabela de Huffman com os códigos binários dos caracteres
void create_huffman_table(NODE* root, char* path, int depth, char* huff_table[256]) {
    if (!root) return;

    if (!root->left && !root->right) {
        path[depth] = '\0';
        huff_table[root->character] = strdup(path);
        return;
    }

    path[depth] = '0';
    create_huffman_table(root->left, path, depth + 1, huff_table);

    path[depth] = '1';
    create_huffman_table(root->right, path, depth + 1, huff_table);
}

// Calcula quantos bits totais serão escritos no corpo compactado
int calculate_bits_trashed(PRIORITY_QUEUE* pq, char* huff_table[256]) {
    int bit_amount = 0;

    while (pq->size > 0) {
        NODE* node = remove_lower(pq);
        if (node && huff_table[node->character]) {
            bit_amount += node->frequency * strlen(huff_table[node->character]);
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
void write_header(PRIORITY_QUEUE* pq, char* huff_table[256], FILE *output_file, NODE* root) {

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

void bit_buffer_add(BitBuffer *buffer, int bit) {
    buffer->byte <<= 1;
    buffer->byte |= (bit & 1);
    buffer->bits_used++;
}

void write_buffer(FILE *f, BitBuffer *buffer) {
    if (buffer->bits_used == 0) return;

    buffer->byte <<= (8 - buffer->bits_used);
    fwrite(&buffer->byte, 1, 1, f);

    buffer->byte = 0;
    buffer->bits_used = 0;
}

// Escreve os dados compactados no novo arquivo
void compactor(FILE *input_file, FILE *output_file, char* huff_table[256]) {
    unsigned char c;
    BitBuffer buffer = {0, 0};

    while (fread(&c, 1, 1, input_file) == 1) {
        char *code = huff_table[c];

        // Verificação para evitar segmentation fault
        if (code == NULL) continue;

        for (int i = 0; code[i] != '\0'; i++) {
            bit_buffer_add(&buffer, code[i] == '1');
            if (buffer.bits_used == 8) {
                write_buffer(output_file, &buffer);
            }
        }
    }

    // Escreve os últimos bits restantes
    write_buffer(output_file, &buffer);
}


// Imprime a tabela de Huffman
void print_huff_table(char* huff_table[256]) {
    for (int i = 0; i < 256; i++) {
        if (huff_table[i]) {
            printf("'%c' (%d): %s\n", i, i, huff_table[i]);
        }
    }
}

#endif // HUFFMAN_H
