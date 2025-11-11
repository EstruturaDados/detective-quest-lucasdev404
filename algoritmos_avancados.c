#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NOME 64
#define MAX_PISTA 128
#define HASH_SIZE 101    // tamanho da tabela hash (primo razoável)

// -------------------------
// ESTRUTURAS
// -------------------------

// Nó da árvore binária que representa uma sala
typedef struct Sala {
    char nome[MAX_NOME];
    char pista[MAX_PISTA];   // pista associada (pode ser vazio)
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// Nó da BST que armazena pistas coletadas (chaves: strings)
typedef struct PistaNode {
    char pista[MAX_PISTA];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// Entrada da tabela hash: chave = pista, valor = suspeito
typedef struct HashEntry {
    char pista[MAX_PISTA];
    char suspeito[MAX_NOME];
    struct HashEntry *proximo;
} HashEntry;

// Tabela hash (array de ponteiros para listas encadeadas)
typedef struct HashTable {
    HashEntry *buckets[HASH_SIZE];
} HashTable;

// Estrutura para contar pistas por suspeito
typedef struct {
    char suspeito[MAX_NOME];
    int cont;
} SuspeitoCount;

// -------------------------
// PROTÓTIPOS
// -------------------------

// Salas
Sala* criarSala(const char* nome, const char* pista);
void liberarSalas(Sala* sala);

// BST de pistas
PistaNode* inserirPista(PistaNode* raiz, const char* pista);
void exibirPistasInOrder(PistaNode* raiz);
void liberarPistas(PistaNode* raiz);

// Hash de pista -> suspeito
unsigned long hashString(const char* str);
HashTable* criarHash();
void inserirNaHash(HashTable* ht, const char* pista, const char* suspeito);
const char* encontrarSuspeito(HashTable* ht, const char* pista);
void liberarHash(HashTable* ht);

// Exploração e verificação
void explorarSalas(Sala* raiz, PistaNode** arvorePistas);
void verificarSuspeitoFinal(PistaNode* arvorePistas, HashTable* ht);

// Auxiliares
void contarPistasPorSuspeitoRec(PistaNode* node, HashTable* ht, SuspeitoCount **lista, size_t *listaSize, size_t *listaCap);
int strcaseequal(const char* a, const char* b);
void limparEntrada();
void trimNewline(char* s);

// -------------------------
// IMPLEMENTAÇÃO
// -------------------------

/*
 * criarSala() – cria dinamicamente um cômodo com nome e pista.
 */
Sala* criarSala(const char* nome, const char* pista) {
    Sala* s = (Sala*) malloc(sizeof(Sala));
    if (!s) {
        fprintf(stderr, "Erro de alocação para sala\n");
        exit(1);
    }
    strncpy(s->nome, nome, MAX_NOME-1);
    s->nome[MAX_NOME-1] = '\0';
    if (pista)
        strncpy(s->pista, pista, MAX_PISTA-1);
    else
        s->pista[0] = '\0';
    s->pista[MAX_PISTA-1] = '\0';
    s->esquerda = s->direita = NULL;
    return s;
}

void liberarSalas(Sala* sala) {
    if (!sala) return;
    liberarSalas(sala->esquerda);
    liberarSalas(sala->direita);
    free(sala);
}

/*
 * inserirPista() – insere a pista coletada na árvore de pistas (BST).
 * Duplica pistas iguais são ignoradas (sem inserir duplicados).
 */
PistaNode* inserirPista(PistaNode* raiz, const char* pista) {
    if (!pista || strlen(pista) == 0) return raiz; // ignora vazias
    if (raiz == NULL) {
        PistaNode* n = (PistaNode*) malloc(sizeof(PistaNode));
        if (!n) { fprintf(stderr, "Erro de alocação para PistaNode\n"); exit(1); }
        strncpy(n->pista, pista, MAX_PISTA-1);
        n->pista[MAX_PISTA-1] = '\0';
        n->esquerda = n->direita = NULL;
        return n;
    }
    int cmp = strcmp(pista, raiz->pista);
    if (cmp < 0) raiz->esquerda = inserirPista(raiz->esquerda, pista);
    else if (cmp > 0) raiz->direita = inserirPista(raiz->direita, pista);
    // se igual, não insere (evita duplicatas)
    return raiz;
}

void exibirPistasInOrder(PistaNode* raiz) {
    if (!raiz) return;
    exibirPistasInOrder(raiz->esquerda);
    printf(" - %s\n", raiz->pista);
    exibirPistasInOrder(raiz->direita);
}

void liberarPistas(PistaNode* raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esquerda);
    liberarPistas(raiz->direita);
    free(raiz);
}

/*
 * hashString() - função de hash (djb2) para strings.
 */
unsigned long hashString(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % HASH_SIZE;
}

/*
 * criarHash() - inicializa a tabela hash
 */
HashTable* criarHash() {
    HashTable* ht = (HashTable*) malloc(sizeof(HashTable));
    if (!ht) { fprintf(stderr, "Erro de alocação para hash\n"); exit(1); }
    for (int i = 0; i < HASH_SIZE; ++i) ht->buckets[i] = NULL;
    return ht;
}

/*
 * inserirNaHash() – insere associação pista -> suspeito na tabela hash.
 */
void inserirNaHash(HashTable* ht, const char* pista, const char* suspeito) {
    if (!pista || strlen(pista) == 0) return;
    unsigned long idx = hashString(pista);
    HashEntry* cur = ht->buckets[idx];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) {
            strncpy(cur->suspeito, suspeito, MAX_NOME-1);
            cur->suspeito[MAX_NOME-1] = '\0';
            return;
        }
        cur = cur->proximo;
    }
    // não encontrado -> cria novo
    HashEntry* novo = (HashEntry*) malloc(sizeof(HashEntry));
    if (!novo) { fprintf(stderr, "Erro de alocação para HashEntry\n"); exit(1); }
    strncpy(novo->pista, pista, MAX_PISTA-1);
    novo->pista[MAX_PISTA-1] = '\0';
    strncpy(novo->suspeito, suspeito, MAX_NOME-1);
    novo->suspeito[MAX_NOME-1] = '\0';
    novo->proximo = ht->buckets[idx];
    ht->buckets[idx] = novo;
}

/*
 * encontrarSuspeito() – consulta o suspeito correspondente a uma pista.
 */
const char* encontrarSuspeito(HashTable* ht, const char* pista) {
    if (!pista || strlen(pista) == 0) return NULL;
    unsigned long idx = hashString(pista);
    HashEntry* cur = ht->buckets[idx];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0)
            return cur->suspeito;
        cur = cur->proximo;
    }
    return NULL;
}

void liberarHash(HashTable* ht) {
    if (!ht) return;
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashEntry* cur = ht->buckets[i];
        while (cur) {
            HashEntry* tmp = cur;
            cur = cur->proximo;
            free(tmp);
        }
    }
    free(ht);
}

/*
 * explorarSalas() – navega pela árvore e ativa o sistema de pistas.
 * Ao entrar em cada sala, coleta automaticamente a pista (insere na BST).
 */
void explorarSalas(Sala* raiz, PistaNode** arvorePistas) {
    if (!raiz) return;
    Sala* atual = raiz;
    char opcao;
    while (atual) {
        printf("\nVocê está na sala: %s\n", atual->nome);
        if (strlen(atual->pista) > 0) {
            printf("Pista encontrada: \"%s\"\n", atual->pista);
            *arvorePistas = inserirPista(*arvorePistas, atual->pista);
        } else {
            printf("Nenhuma pista nesta sala.\n");
        }

        // opções
        printf("\nEscolha o caminho:\n");
        if (atual->esquerda) printf(" (e) Ir para a esquerda -> %s\n", atual->esquerda->nome);
        if (atual->direita)  printf(" (d) Ir para a direita  -> %s\n", atual->direita->nome);
        printf(" (s) Sair da exploração\n");
        printf("Opção: ");
        if (scanf(" %c", &opcao) != 1) opcao = 's';
        limparEntrada();

        if (opcao == 'e' || opcao == 'E') {
            if (atual->esquerda) atual = atual->esquerda;
            else printf("Não há caminho à esquerda.\n");
        } else if (opcao == 'd' || opcao == 'D') {
            if (atual->direita) atual = atual->direita;
            else printf("Não há caminho à direita.\n");
        } else if (opcao == 's' || opcao == 'S') {
            printf("\nExploração encerrada pelo jogador.\n");
            break;
        } else {
            printf("Opção inválida. Tente novamente.\n");
        }
    }
}

/*
 * Função auxiliar recursiva que percorre a BST de pistas e conta,
 * para cada pista, o suspeito correspondente (usando a hash),
 * atualizando o vetor dinâmico de SuspeitoCount.
 */
void contarPistasPorSuspeitoRec(PistaNode* node, HashTable* ht, SuspeitoCount **lista, size_t *listaSize, size_t *listaCap) {
    if (!node) return;

    contarPistasPorSuspeitoRec(node->esquerda, ht, lista, listaSize, listaCap);

    const char* suspeito = encontrarSuspeito(ht, node->pista);
    if (suspeito) {
        size_t i;
        for (i = 0; i < *listaSize; ++i) {
            if (strcmp((*lista)[i].suspeito, suspeito) == 0) {
                (*lista)[i].cont++;
                break;
            }
        }
        if (i == *listaSize) {
            if (*listaSize == *listaCap) {
                *listaCap = (*listaCap == 0) ? 4 : (*listaCap) * 2;
                SuspeitoCount *tmp = realloc(*lista, (*listaCap) * sizeof(SuspeitoCount));
                if (!tmp) { fprintf(stderr, "Erro de alocação\n"); exit(1); }
                *lista = tmp;
            }
            strncpy((*lista)[*listaSize].suspeito, suspeito, MAX_NOME-1);
            (*lista)[*listaSize].suspeito[MAX_NOME-1] = '\0';
            (*lista)[*listaSize].cont = 1;
            (*listaSize)++;
        }
    }

    contarPistasPorSuspeitoRec(node->direita, ht, lista, listaSize, listaCap);
}

/*
 * verificarSuspeitoFinal() – conduz à fase de julgamento final.
 * Conta quantas pistas coletadas apontam para cada suspeito (usando a hash).
 * Se o suspeito acusado tiver ao menos 2 pistas apontando para ele, acusação confirmada.
 */
void verificarSuspeitoFinal(PistaNode* arvorePistas, HashTable* ht) {
    printf("\n===== PISTAS COLETADAS (ordem alfabética) =====\n");
    if (!arvorePistas) {
        printf("Nenhuma pista coletada.\n");
    } else {
        exibirPistasInOrder(arvorePistas);
    }

    // Cria vetor dinâmico de SuspeitoCount
    SuspeitoCount *lista = NULL;
    size_t listaSize = 0, listaCap = 0;

    contarPistasPorSuspeitoRec(arvorePistas, ht, &lista, &listaSize, &listaCap);

    if (listaSize == 0) {
        printf("\nNenhuma pista corresponde a suspeitos conhecidos.\n");
    } else {
        printf("\nContagem de pistas por suspeito:\n");
        for (size_t i = 0; i < listaSize; ++i) {
            printf(" - %s : %d pista(s)\n", lista[i].suspeito, lista[i].cont);
        }
    }

    char acusacao[MAX_NOME];
    printf("\nDigite o nome do suspeito que deseja acusar: ");
    if (!fgets(acusacao, sizeof(acusacao), stdin)) { acusacao[0] = '\0'; }
    trimNewline(acusacao);
    if (strlen(acusacao) == 0) {
        printf("Nenhum nome informado. Encerrando julgamento.\n");
        free(lista);
        return;
    }

    int contAcusado = 0;
    for (size_t i = 0; i < listaSize; ++i) {
        if (strcaseequal(lista[i].suspeito, acusacao)) {
            contAcusado = lista[i].cont;
            break;
        }
    }

    if (contAcusado >= 2) {
        printf("\nAcusação confirmada! %s tem %d pista(s) apoiando a acusação.\n", acusacao, contAcusado);
        printf("Parabéns, detetive — a investigação apontou evidências suficientes.\n");
    } else if (contAcusado > 0) {
        printf("\nAcusação fraca. %s tem apenas %d pista(s) apoiando a acusação.\n", acusacao, contAcusado);
        printf("Você precisa de pelo menos 2 pistas para confirmar a culpa.\n");
    } else {
        printf("\nNenhuma pista coletada aponta para %s.\n", acusacao);
        printf("A acusação não procede com as evidências disponíveis.\n");
    }

    free(lista);
}

/*
 * Comparação case-insensitive simples (retorna 1 se iguais, 0 caso contrário)
 */
int strcaseequal(const char* a, const char* b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return (*a == '\0' && *b == '\0');
}

void limparEntrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void trimNewline(char* s) {
    size_t n = strlen(s);
    if (n == 0) return;
    if (s[n-1] == '\n') s[n-1] = '\0';
}

// -------------------------
// MAIN: monta o mapa, hash e inicia o fluxo
// -------------------------

int main() {
    // Montagem estática da mansão (árvore binária)
    Sala* hall = criarSala("Hall de Entrada", "Pegadas de lama no tapete");
    hall->esquerda = criarSala("Sala de Estar", "Um copo quebrado no chão");
    hall->direita = criarSala("Biblioteca", "Livro aberto com anotações sobre 'experimentos'");

    hall->esquerda->esquerda = criarSala("Cozinha", "Luvas de borracha descartadas");
    hall->esquerda->direita  = criarSala("Sala de Jantar", "Talher com impressões digitais");
    hall->direita->esquerda  = criarSala("Jardim", "Pegadas levando ao portão dos fundos");
    hall->direita->direita   = criarSala("Laboratório Secreto", "Frascos de substância química estranha");

    // Monta a tabela hash com associação pista -> suspeito
    HashTable* ht = criarHash();
    inserirNaHash(ht, "Pegadas de lama no tapete", "Sr. Oliveira");
    inserirNaHash(ht, "Um copo quebrado no chão", "Sra. Almeida");
    inserirNaHash(ht, "Livro aberto com anotações sobre 'experimentos'", "Dr. Mendes");
    inserirNaHash(ht, "Luvas de borracha descartadas", "Sra. Almeida");
    inserirNaHash(ht, "Talher com impressões digitais", "Sr. Oliveira");
    inserirNaHash(ht, "Pegadas levando ao portão dos fundos", "Sr. Oliveira");
    inserirNaHash(ht, "Frascos de substância química estranha", "Dr. Mendes");

    // Árvore de pistas coletadas (inicialmente vazia)
    PistaNode* arvorePistas = NULL;

    printf("=== Detective Quest: Investigação Final ===\n");
    printf("Explore a mansão, colete pistas e acuse o culpado.\n");

    // Exploração interativa (coleta automática de pistas)
    explorarSalas(hall, &arvorePistas);

    // Verificação final do suspeito
    verificarSuspeitoFinal(arvorePistas, ht);

    // Limpeza
    liberarSalas(hall);
    liberarPistas(arvorePistas);
    liberarHash(ht);

    printf("\nEncerrando investigação. Obrigado por jogar!\n");
    return 0;
}
