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
    if (original_file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Create the name of the file to be created
    char new_file_name[BUFFER_SIZE + 5];
    snprintf(new_file_name, sizeof(new_file_name), "%s.huff", filename);

    FILE *new_file = fopen(new_file_name, "wb");
    if (new_file == NULL) {
        perror("Error creating output file");
        fclose(original_file);
        return 1;
    }

    // Cria as duas filas de prioridade
    PRIORITY_QUEUE* huff_queue1 = create_queue();
    PRIORITY_QUEUE* huff_queue2 = create_queue();

    // Preenche as filas com as frequências dos caracteres do arquivo
    create_huff_queue(original_file, &huff_queue1, &huff_queue2);

    // Constrói a árvore de Huffman
    NODE* root = build_huffman_tree(huff_queue1);

    // Cria a tabela de códigos de Huffman
    char* huff_table[256] = { NULL };
    char path[256];
    create_huffman_table(root, path, 0, huff_table);

    // Escreve o cabeçalho e a árvore no novo arquivo
    write_header(huff_queue2, huff_table, new_file, root);

    // Reposiciona o ponteiro do arquivo original para o início
    rewind(original_file);

    // Compacta os dados do arquivo original usando a tabela de Huffman
    //compactor(original_file, new_file, huff_table);

    // Fecha os arquivos
    fclose(original_file);
    fclose(new_file);

    // Libera a memória da tabela de Huffman
    for (int i = 0; i < 256; i++) {
        if (huff_table[i]) {
            free(huff_table[i]);
        }
    }

    printf("Arquivo compactado com sucesso: %s\n", new_file_name);
    return 0;
}
