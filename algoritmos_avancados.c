#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========================
// ESTRUTURAS DE DADOS
// ========================

// Estrutura para representar uma sala na mansão
typedef struct Sala {
    char nome[50];             // Nome do cômodo
    char pista[100];           // Pista encontrada (pode estar vazia)
    struct Sala *esquerda;     // Caminho à esquerda
    struct Sala *direita;      // Caminho à direita
} Sala;

// Estrutura para representar um nó da árvore de pistas (BST)
typedef struct PistaNode {
    char pista[100];           // Conteúdo da pista
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// ========================
// FUNÇÕES DE SALAS
// ========================

/*
 * Função: criarSala
 * -----------------
 * Cria dinamicamente uma nova sala com nome e pista opcional.
 * Retorna um ponteiro para a sala criada.
 */
Sala* criarSala(const char* nome, const char* pista) {
    Sala* nova = (Sala*) malloc(sizeof(Sala));
    if (nova == NULL) {
        printf("Erro ao alocar memória para a sala.\n");
        exit(1);
    }
    strcpy(nova->nome, nome);
    strcpy(nova->pista, pista);
    nova->esquerda = NULL;
    nova->direita = NULL;
    return nova;
}

// ========================
// FUNÇÕES DA BST DE PISTAS
// ========================

/*
 * Função: inserirPista
 * --------------------
 * Insere uma nova pista na árvore BST de forma ordenada.
 */
PistaNode* inserirPista(PistaNode* raiz, const char* novaPista) {
    if (novaPista == NULL || strlen(novaPista) == 0) return raiz; // ignora pistas vazias

    if (raiz == NULL) {
        PistaNode* novo = (PistaNode*) malloc(sizeof(PistaNode));
        strcpy(novo->pista, novaPista);
        novo->esquerda = NULL;
        novo->direita = NULL;
        return novo;
    }

    if (strcmp(novaPista, raiz->pista) < 0)
        raiz->esquerda = inserirPista(raiz->esquerda, novaPista);
    else if (strcmp(novaPista, raiz->pista) > 0)
        raiz->direita = inserirPista(raiz->direita, novaPista);
    // Se for igual, ignora (sem duplicatas)

    return raiz;
}

/*
 * Função: exibirPistas
 * --------------------
 * Exibe todas as pistas coletadas em ordem alfabética (in-order traversal).
 */
void exibirPistas(PistaNode* raiz) {
    if (raiz == NULL) return;
    exibirPistas(raiz->esquerda);
    printf(" - %s\n", raiz->pista);
    exibirPistas(raiz->direita);
}

/*
 * Função: liberarPistas
 * ---------------------
 * Libera a memória ocupada pela árvore de pistas.
 */
void liberarPistas(PistaNode* raiz) {
    if (raiz == NULL) return;
    liberarPistas(raiz->esquerda);
    liberarPistas(raiz->direita);
    free(raiz);
}

// ========================
// FUNÇÃO DE EXPLORAÇÃO
// ========================

/*
 * Função: explorarSalasComPistas
 * ------------------------------
 * Permite ao jogador explorar a mansão e coletar pistas automaticamente.
 */
void explorarSalasComPistas(Sala* salaAtual, PistaNode** arvorePistas) {
    char escolha;

    while (salaAtual != NULL) {
        printf("\nVocê está na: %s\n", salaAtual->nome);

        // Coleta automática da pista, se existir
        if (strlen(salaAtual->pista) > 0) {
            printf("Você encontrou uma pista: \"%s\"\n", salaAtual->pista);
            *arvorePistas = inserirPista(*arvorePistas, salaAtual->pista);
        } else {
            printf("Não há nenhuma pista nesta sala.\n");
        }

        printf("\nEscolha um caminho:\n");
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
            printf("\nExploração encerrada. Vamos analisar as pistas!\n");
            break;
        }
        else {
            printf("Opção inválida! Tente novamente.\n");
        }
    }
}

// ========================
// LIBERAÇÃO DE MEMÓRIA
// ========================

void liberarSalas(Sala* sala) {
    if (sala == NULL) return;
    liberarSalas(sala->esquerda);
    liberarSalas(sala->direita);
    free(sala);
}

// ========================
// FUNÇÃO PRINCIPAL
// ========================

int main() {
    // Montagem automática da mansão (árvore binária)
    Sala* hall = criarSala("Hall de Entrada", "Pegadas de lama no tapete.");
    hall->esquerda = criarSala("Sala de Estar", "Um copo quebrado no chão.");
    hall->direita = criarSala("Biblioteca", "Um livro aberto com anotações suspeitas.");

    hall->esquerda->esquerda = criarSala("Cozinha", "Uma luva de borracha faltando o par.");
    hall->esquerda->direita = criarSala("Sala de Jantar", "");
    hall->direita->esquerda = criarSala("Jardim", "Pegadas levando ao portão dos fundos.");
    hall->direita->direita = criarSala("Laboratório Secreto", "Frascos de substância química estranha.");

    // Ponteiro para a árvore de pistas
    PistaNode* arvorePistas = NULL;

    printf("=== Detective Quest: Coleta de Pistas ===\n");
    printf("Explore a mansão e descubra os segredos escondidos!\n");

    // Inicia a exploração e coleta de pistas
    explorarSalasComPistas(hall, &arvorePistas);

    // Exibe todas as pistas coletadas em ordem alfabética
    printf("\n===== PISTAS COLETADAS =====\n");
    if (arvorePistas == NULL)
        printf("Nenhuma pista foi coletada.\n");
    else
        exibirPistas(arvorePistas);

    // Liberação de memória
    liberarSalas(hall);
    liberarPistas(arvorePistas);

    printf("\nFim da investigação. Boa sorte, detetive!\n");
    return 0;
}
