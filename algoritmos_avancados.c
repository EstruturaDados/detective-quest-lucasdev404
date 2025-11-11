#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura que representa uma sala da mansão
typedef struct Sala {
    char nome[50];              // Nome do cômodo
    struct Sala *esquerda;      // Caminho à esquerda
    struct Sala *direita;       // Caminho à direita
} Sala;

/*
 * Função: criarSala
 * -----------------
 * Cria dinamicamente uma nova sala com o nome informado.
 * Retorna o ponteiro para a nova sala criada.
 */
Sala* criarSala(const char* nome) {
    Sala* nova = (Sala*) malloc(sizeof(Sala));
    if (nova == NULL) {
        printf("Erro ao alocar memória para a sala.\n");
        exit(1);
    }
    strcpy(nova->nome, nome);
    nova->esquerda = NULL;
    nova->direita = NULL;
    return nova;
}

/*
 * Função: explorarSalas
 * ---------------------
 * Permite que o jogador explore a mansão interativamente.
 * O jogador escolhe ir para a esquerda ('e'), direita ('d') ou sair ('s').
 * A exploração termina quando o jogador chega a uma sala sem caminhos.
 */
void explorarSalas(Sala* salaAtual) {
    char escolha;

    while (salaAtual != NULL) {
        printf("\nVocê está na: %s\n", salaAtual->nome);

        // Se for uma sala sem caminhos, fim da exploração
        if (salaAtual->esquerda == NULL && salaAtual->direita == NULL) {
            printf("Não há mais caminhos a seguir. Você chegou ao fim da exploração!\n");
            break;
        }

        printf("Escolha um caminho:\n");
        if (salaAtual->esquerda != NULL)
            printf(" (e) Esquerda -> %s\n", salaAtual->esquerda->nome);
        if (salaAtual->direita != NULL)
            printf(" (d) Direita  -> %s\n", salaAtual->direita->nome);
        printf(" (s) Sair da exploração\n");
        printf("Opção: ");
        scanf(" %c", &escolha);

        if (escolha == 'e' || escolha == 'E') {
            if (salaAtual->esquerda != NULL)
                salaAtual = salaAtual->esquerda;
            else
                printf("Não há caminho à esquerda!\n");
        }
        else if (escolha == 'd' || escolha == 'D') {
            if (salaAtual->direita != NULL)
                salaAtual = salaAtual->direita;
            else
                printf("Não há caminho à direita!\n");
        }
        else if (escolha == 's' || escolha == 'S') {
            printf("Exploração encerrada. Até a próxima, detetive!\n");
            break;
        }
        else {
            printf("Opção inválida! Escolha novamente.\n");
        }
    }
}

/*
 * Função: liberarSalas
 * --------------------
 * Libera a memória ocupada pela árvore binária de salas (recursivamente).
 */
void liberarSalas(Sala* sala) {
    if (sala == NULL) return;
    liberarSalas(sala->esquerda);
    liberarSalas(sala->direita);
    free(sala);
}

/*
 * Função principal: main
 * ----------------------
 * Monta o mapa da mansão (árvore binária) e inicia a exploração.
 */
int main() {
    // Montagem da árvore de forma automática
    Sala* hall = criarSala("Hall de Entrada");
    hall->esquerda = criarSala("Sala de Estar");
    hall->direita = criarSala("Biblioteca");

    hall->esquerda->esquerda = criarSala("Cozinha");
    hall->esquerda->direita = criarSala("Sala de Jantar");

    hall->direita->esquerda = criarSala("Jardim");
    hall->direita->direita = criarSala("Laboratório Secreto");

    printf("=== Detective Quest: Mapa da Mansão ===\n");
    printf("Você está prestes a explorar a mansão misteriosa!\n");

    explorarSalas(hall);

    liberarSalas(hall);
    return 0;
}
