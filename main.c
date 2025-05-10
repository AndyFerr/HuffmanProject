#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>  // Para uint32_t
#include <ctype.h>   // Para validações de entrada, se desejar
#include "huffman.h"
#include "pqueue_heap.h"

#define BUFFER_SIZE 1000

// Protótipo da função de descompactação (a ser implementada futuramente)
void decompressor(const char* compressed_filename);

int main() {
    int option;
    
    printf("====== Compactador Huffman ======\n");
    printf("Escolha uma opção:\n");
    printf("1 - Compactar arquivo\n");
    printf("2 - Descompactar arquivo\n");
    printf("Opção: ");
    scanf("%d", &option);
    getchar(); // Limpa o buffer do ENTER

    if (option == 1) {
        printf("\nInsira o nome do arquivo a ser compactado:\n");

        char filename[BUFFER_SIZE];
        scanf("%s", filename);

        FILE* original_file = fopen(filename, "rb");
        if (original_file == NULL) {
            perror("Erro ao abrir o arquivo");
            return 1;
        }

        // Remove a extensão do nome do arquivo
        char base_name[BUFFER_SIZE];
        strcpy(base_name, filename);

        char* dot = strrchr(base_name, '.');
        if (dot != NULL) {
            *dot = '\0'; // termina a string antes do ponto
        }

        // Cria um novo nome com extensão .huff
        char new_file_name[BUFFER_SIZE];
        snprintf(new_file_name, sizeof(new_file_name), "%s.huff", base_name);

        FILE *new_file = fopen(new_file_name, "wb");
        if (new_file == NULL) {
            perror("Erro ao criar o arquivo de saída");
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
        HuffmanCode huff_table[256] = {0}; // Inicializa a tabela de Huffman
        uint32_t code = 0; // Inicializa o código como um inteiro
        create_huffman_table(root, code, 0, huff_table); // Passa o código como ponteiro

        // Escreve o cabeçalho e a árvore no novo arquivo
        write_header(huff_queue2, huff_table, new_file, root);

        // Libera memória usada
        free_huffman_tree(root);
        free_priority_queue(huff_queue1);
        free_priority_queue(huff_queue2);

        // Reposiciona o ponteiro do arquivo original para o início
        rewind(original_file);

        // Compacta os dados do arquivo original usando a tabela de Huffman
        compactor(original_file, new_file, huff_table);

        // Fecha os arquivos
        fclose(original_file);
        fclose(new_file);

        printf("Arquivo compactado com sucesso: %s\n", new_file_name);

    } else if (option == 2) {
        char compressed_filename[BUFFER_SIZE];
        printf("\nInsira o nome do arquivo compactado (.huff):\n");
        scanf("%s", compressed_filename);

        // Chamada da função de descompactação (ainda a ser implementada)
        //decompressor(compressed_filename);

    } else {
        printf("Opção inválida.\n");
    }

    return 0;
}

