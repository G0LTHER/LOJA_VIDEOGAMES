/*
===================================================================================
             PROJETO — LOJA DE VIDEOGAMES (C PURO, compatível C89)
-----------------------------------------------------------------------------------
Versão didática 2025 - baseada no ProjetoFinal_C.c enviado pelo usuário
===================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

/* -------------------------------------------------------------------------------
   Estrutura de dados: formato exato de cada registro no arquivo binário
---------------------------------------------------------------------------------*/
typedef struct {
    char titulo[60];
    char plataforma[20];
    char genero[30];
    float preco;
    int estoque;
    char status; /* ' ' = ativo, '*' = excluído logicamente */
} jogo;

/* Protótipos */
void configurar_locale(void);
void limpa_buffer(void);
void ler_string(char *s, int tam);
int tamanho(FILE *arq);
void cadastrar(FILE *arq);
void consultar(FILE *arq);
void gerar_arquivo_texto(FILE *arq);
void vender(FILE *arq);
void reposicao(FILE *arq);
void excluir(FILE *arq);

/* ------------------------------------------------------------------------------- */
void limpa_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void ler_string(char *s, int tam) {
    fgets(s, tam, stdin);
    s[strcspn(s, "\n")] = '\0';
}

int tamanho(FILE *arq) {
    long pos = ftell(arq);
    fseek(arq, 0, SEEK_END);
    long fim = ftell(arq);
    fseek(arq, pos, SEEK_SET);
    return (int)(fim / sizeof(jogo));
}

/* Cadastrar novo jogo */
void cadastrar(FILE *arq) {
    jogo item;
    char confirma;
    int total;

    item.status = ' ';
    printf("\n=== CADASTRAR JOGO ===\n");
    total = tamanho(arq);
    printf("Próximo código: %d\n", total + 1);

    printf("Título: ");
    ler_string(item.titulo, sizeof(item.titulo));

    printf("Plataforma (ex: PS5, Switch, PC): ");
    ler_string(item.plataforma, sizeof(item.plataforma));

    printf("Gênero: ");
    ler_string(item.genero, sizeof(item.genero));

    printf("Preço (use ponto para decimais): ");
    if (scanf("%f", &item.preco) != 1) {
        printf("Preço inválido. Cadastro cancelado.\n");
        limpa_buffer();
        return;
    }

    printf("Quantidade em estoque: ");
    if (scanf("%d", &item.estoque) != 1) {
        printf("Quantidade inválida. Cadastro cancelado.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    printf("Confirmar cadastro (s/n)? ");
    if (scanf("%c", &confirma) != 1) {
        printf("Entrada inválida. Cancelando cadastro.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    if (toupper(confirma) == 'S') {
        fseek(arq, 0, SEEK_END);
        fwrite(&item, sizeof(jogo), 1, arq);
        fflush(arq);
        printf("Jogo cadastrado com sucesso!\n");
    } else {
        printf("Cadastro cancelado.\n");
    }
}

/* Consultar por código (acesso direto) */
void consultar(FILE *arq) {
    int nr;
    jogo item;
    int total;

    printf("\nInforme o código do jogo: ");
    if (scanf("%d", &nr) != 1) {
        printf("Entrada inválida!\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    total = tamanho(arq);
    if (nr <= 0 || nr > total) {
        printf("Código inválido! Total de registros = %d\n", total);
        return;
    }

    if (fseek(arq, (long)(nr - 1) * sizeof(jogo), SEEK_SET) != 0) {
        printf("Erro ao posicionar arquivo.\n");
        return;
    }

    if (fread(&item, sizeof(jogo), 1, arq) != 1) {
        printf("Erro ao ler registro.\n");
        return;
    }

    printf("\n=== JOGO (CÓDIGO %d) ===\n", nr);
    if (item.status == '*') {
        printf("Status: ** EXCLUÍDO LOGICAMENTE **\n");
    }
    printf("Título.....: %s\n", item.titulo);
    printf("Plataforma.: %s\n", item.plataforma);
    printf("Gênero.....: %s\n", item.genero);
    printf("Preço......: R$ %.2f\n", item.preco);
    printf("Estoque....: %d\n", item.estoque);
}

/* Gera arquivo texto com relatório completo */
void gerar_arquivo_texto(FILE *arq) {
    char nomearq[80];
    jogo item;
    int i;
    int total;
    char status_str[12];

    printf("\nGerar Relatório em Arquivo Texto\n");
    printf("Nome do arquivo (sem extensão): ");
    ler_string(nomearq, sizeof(nomearq));
    strcat(nomearq, ".txt");

    FILE *arqtxt = fopen(nomearq, "w");
    if (!arqtxt) {
        printf("Erro ao criar arquivo texto.\n");
        return;
    }

    fprintf(arqtxt, "RELATÓRIO - LOJA DE VIDEOGAMES\n\n");
    fprintf(arqtxt, "COD  %-30s %-10s %-10s %8s %7s  %s\n",
            "TÍTULO", "PLATAFORMA", "GÊNERO", "PREÇO", "ESTOQUE", "STATUS");
    fprintf(arqtxt, "----------------------------------------------------------------------------------------\n");

    total = tamanho(arq);
    for (i = 0; i < total; i++) {
        fseek(arq, i * sizeof(jogo), SEEK_SET);
        fread(&item, sizeof(jogo), 1, arq);
        if (item.status == '*')
            strcpy(status_str, "EXCLUÍDO");
        else
            strcpy(status_str, "ATIVO");
        fprintf(arqtxt, "%03d  %-30s %-10s %-10s %8.2f %7d  %s\n",
                i + 1,
                item.titulo,
                item.plataforma,
                item.genero,
                item.preco,
                item.estoque,
                status_str);
    }

    fclose(arqtxt);
    printf("Arquivo '%s' gerado com sucesso!\n", nomearq);
}

/* Vender um jogo: diminui estoque (se disponível) e mostra valor total */
void vender(FILE *arq) {
    int nr;
    int qtd;
    jogo item;
    int total;
    char confirma;

    printf("\nVender jogo - informe o código: ");
    if (scanf("%d", &nr) != 1) {
        printf("Entrada inválida!\n");
        limpa_buffer();
        return;
    }

    limpa_buffer();
    total = tamanho(arq);
    if (nr <= 0 || nr > total) {
        printf("Código inválido. Total de registros = %d\n", total);
        return;
    }

    fseek(arq, (long)(nr - 1) * sizeof(jogo), SEEK_SET);
    if (fread(&item, sizeof(jogo), 1, arq) != 1) {
        printf("Erro ao ler registro.\n");
        return;
    }

    if (item.status == '*') {
        printf("Este registro está logicamente excluído e não pode ser vendido.\n");
        return;
    }

    printf("Título: %s\n", item.titulo);
    printf("Estoque atual: %d\n", item.estoque);
    printf("Quantidade a vender: ");
    if (scanf("%d", &qtd) != 1) {
        printf("Quantidade inválida.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    if (qtd <= 0) {
        printf("Quantidade deve ser maior que zero.\n");
        return;
    }
    if (qtd > item.estoque) {
        printf("Estoque insuficiente para a venda.\n");
        return;
    }

    printf("Confirmar venda de %d unidade(s) do jogo '%s' (s/n)? ", qtd, item.titulo);
    if (scanf("%c", &confirma) != 1) {
        printf("Entrada inválida.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    if (toupper(confirma) == 'S') {
        item.estoque -= qtd;
        fseek(arq, (long)(nr - 1) * sizeof(jogo), SEEK_SET);
        fwrite(&item, sizeof(jogo), 1, arq);
        fflush(arq);
        printf("Venda realizada com sucesso! Total: R$ %.2f\n", item.preco * qtd);
        printf("Estoque atualizado: %d\n", item.estoque);
    } else {
        printf("Venda cancelada.\n");
    }
}

void reposicao(FILE *arq) {
    int nr;
    int qtd;
    jogo item;
    int total;
    char confirma;

    printf("\nReposição de estoque - informe o código: ");
    if (scanf("%d", &nr) != 1) {
        printf("Entrada inválida!\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    total = tamanho(arq);
    if (nr <= 0 || nr > total) {
        printf("Código inválido. Total de registros = %d\n", total);
        return;
    }

    fseek(arq, (long)(nr - 1) * sizeof(jogo), SEEK_SET);
    if (fread(&item, sizeof(jogo), 1, arq) != 1) {
        printf("Erro ao ler registro.\n");
        return;
    }

    if (item.status == '*') {
        printf("Registro logicamente excluído. Não é possível repor o estoque.\n");
        return;
    }

    printf("Título: %s\n", item.titulo);
    printf("Estoque atual: %d\n", item.estoque);
    printf("Quantidade a adicionar: ");
    if (scanf("%d", &qtd) != 1) {
        printf("Quantidade inválida.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    if (qtd <= 0) {
        printf("Quantidade deve ser maior que zero.\n");
        return;
    }

    printf("Confirmar reposição de %d unidade(s) para '%s' (s/n)? ", qtd, item.titulo);
    if (scanf("%c", &confirma) != 1) {
        printf("Entrada inválida.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    if (toupper(confirma) == 'S') {
        item.estoque += qtd;
        fseek(arq, (long)(nr - 1) * sizeof(jogo), SEEK_SET);
        fwrite(&item, sizeof(jogo), 1, arq);
        fflush(arq);
        printf("Estoque reposto com sucesso!\n");
        printf("Estoque atualizado: %d\n", item.estoque);
    } else {
        printf("Reposição cancelada.\n");
    }
}

void excluir(FILE *arq) {
    int nr;
    char confirma;
    jogo item;
    int total;

    printf("\nInforme o código do registro para exclusão: ");
    if (scanf("%d", &nr) != 1) {
        printf("Entrada inválida!\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    total = tamanho(arq);
    if (nr <= 0 || nr > total) {
        printf("Código inválido. Total de registros = %d\n", total);
        return;
    }

    fseek(arq, (long)(nr - 1) * sizeof(jogo), SEEK_SET);
    if (fread(&item, sizeof(jogo), 1, arq) != 1) {
        printf("Erro ao ler registro.\n");
        return;
    }

    if (item.status == '*') {
        printf("Registro já estava logicamente excluído.\n");
        return;
    }

    printf("Confirmar exclusão lógica do jogo '%s' (s/n)? ", item.titulo);
    if (scanf("%c", &confirma) != 1) {
        printf("Entrada inválida.\n");
        limpa_buffer();
        return;
    }
    limpa_buffer();

    if (toupper(confirma) == 'S') {
        item.status = '*';
        fseek(arq, (long)(nr - 1) * sizeof(jogo), SEEK_SET);
        fwrite(&item, sizeof(jogo), 1, arq);
        fflush(arq);
        printf("Registro excluído logicamente com sucesso!\n");
    } else {
        printf("Exclusão cancelada.\n");
    }
}

/* Configura locale e o console para UTF-8 */
void configurar_locale(void) {
    #if defined(_WIN32)
    /* ESSA LINHA É CRUCIAL PARA CORRIGIR A EXIBIÇÃO NO CONSOLE DO WINDOWS */
    system("chcp 65001 > nul"); 
    #endif

    {
        const char *locais[] = {
            "pt_BR.UTF-8",
            "pt_BR.utf8",
            "Portuguese_Brazil.1252",
            "Portuguese",
            ""
        };
        int i;
        for (i = 0; i < 5; i++) {
            const char *r = setlocale(LC_ALL, locais[i]);
            if (r != NULL) {
                printf("Locale ativo: %s\n", r);
                return;
            }
        }
        printf("Aviso: Locale não pode ser configurado.\n");
    }
}

/* main */
int main(void) {
    FILE *arq;
    int op;

    // A chamada aqui é essencial!
    configurar_locale();

    arq = fopen("loja.dat", "r+b");
    if (!arq) {
        arq = fopen("loja.dat", "w+b");
        if (!arq) {
            printf("Erro crítico ao abrir/criar arquivo de dados.\n");
            return 1;
        }
    }

    do {
        printf("\n====== LOJA DE VIDEOGAMES ======\n");
        printf("1. Cadastrar jogo\n");
        printf("2. Consultar por código\n");
        printf("3. Gerar relatório (.txt)\n");
        printf("4. Vender (descontar estoque)\n");
        printf("5. Repor estoque\n");
        printf("6. Excluir registro (lógico)\n");
        printf("7. Sair\n");
        printf("-------------------------------\n");
        printf("Total de registros: %d\n", tamanho(arq));
        printf("Opção: ");

        if (scanf("%d", &op) != 1) {
            printf("Digite um número válido.\n");
            limpa_buffer();
            continue;
        }
        limpa_buffer();

        switch (op) {
            case 1: cadastrar(arq); break;
            case 2: consultar(arq); break;
            case 3: gerar_arquivo_texto(arq); break;
            case 4: vender(arq); break;
            case 5: reposicao(arq); break;
            case 6: excluir(arq); break;
            case 7: printf("Fechando arquivo e saindo...\n"); break;
            default: printf("Opção inválida!\n");
        }
    } while (op != 7);

    fclose(arq);
    return 0;
}
