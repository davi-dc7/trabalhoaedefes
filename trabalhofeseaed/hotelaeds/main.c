#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQ_CLIENTES "clientes.dat"
#define ARQ_FUNCIONARIOS "funcionarios.dat"
#define ARQ_QUARTOS "quartos.dat"
#define ARQ_ESTADIAS "estadias.dat"

typedef struct {
    int codigo;
    char nome[80];
    char endereco[120];
    char telefone[20];
} Cliente;

typedef struct {
    int codigo;
    char nome[80];
    char telefone[20];
    char cargo[30];
    float salario;
} Funcionario;

typedef struct {
    int numero;
    int qtdHospedes;
    float valorDiaria;
    int ocupado;
} Quarto;

typedef struct {
    int codigo;
    // Tamanho 9 para 8 dígitos (AAAAMMDD) + \0
    char dataEntrada[9];
    char dataSaida[9];
    int qtdDiarias;
    int codCliente;
    int numeroQuarto;
    int ativo;
} Estadia;

//funcoes utilitarias

// Lida com newline de fgets
void trim_newline(char *s) {
    size_t n = strlen(s);
    if (n > 0 && s[n-1] == '\n') s[n-1] = '\0';
}

// Funcao robusta para limpar o buffer (resolve problemas de scanf seguido de fgets)
void limpar_buffer_scanf() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

// NOVO: Lê dia, mês e ano separadamente, valida, e formata em AAAAMMDD
// Retorna 1 em sucesso, 0 em falha.
int ler_e_formatar_data(const char *prompt, char *destino_AAAAMMDD) {
    int d, m, a;
    printf("%s (Dia Mes Ano): ", prompt);
    // Lê dia, mês, ano
    if (scanf("%d %d %d", &d, &m, &a) != 3) {
        printf("Formato de data invalido.\n");
        limpar_buffer_scanf();
        return 0; // Falha na leitura
    }
    limpar_buffer_scanf(); // Limpa o buffer após o scanf

    // Validação de formato simplificada
    if (a < 2024 || m < 1 || m > 12 || d < 1 || d > 31) {
        printf("Data invalida. (Ano deve ser >= 2024, Mes 1-12, Dia 1-31).\n");
        return 0;
    }

    // Formata para o formato AAAAMMDD para uso na struct e comparações
    sprintf(destino_AAAAMMDD, "%04d%02d%02d", a, m, d);
    return 1;
}


/* compara datas no formato "AAAAMMDD" lexicograficamente:
   retorna <0 se a < b, 0 se iguais, >0 se a > b */
int cmp_date(const char *a, const char *b) {
    return strcmp(a, b);
}

/* calcula diferenca de dias simplificada contando dias inteiros:
   assume formato AAAAMMDD valido e que b > a
*/
int diff_days_simple(const char *entrada, const char *saida) {
    int ay, am, ad, by, bm, bd;
    // Lendo AAAAMMDD (4 digitos ano, 2 mes, 2 dia)
    if (sscanf(entrada, "%4d%2d%2d", &ay, &am, &ad) != 3) return -1;
    if (sscanf(saida, "%4d%2d%2d", &by, &bm, &bd) != 3) return -1;
    int dias = (by - ay) * 365 + (bm - am) * 30 + (bd - ad);
    if (dias <= 0) return -1;
    return dias;
}

/* verifica se dois intervalos [a1,a2) e [b1,b2) se sobrepoem.
   usamos comparacao lexicografica pois as datas estao em AAAAMMDD.
   retorna 1 se overlap, 0 se nao overlap.
*/
int sobrepoe(const char *a1, const char *a2, const char *b1, const char *b2) {
    if (cmp_date(a2, b1) <= 0) return 0;
    if (cmp_date(b2, a1) <= 0) return 0;
    return 1;
}

//gera codigo automatico (max + 1) lendo arquivo de registro
int gerar_codigo_cliente() {
    FILE *f = fopen(ARQ_CLIENTES, "rb");
    if (!f) return 1;
    Cliente c;
    int max = 0;
    while (fread(&c, sizeof(Cliente), 1, f) == 1) {
        if (c.codigo > max) max = c.codigo;
    }
    fclose(f);
    return max + 1;
}
int gerar_codigo_funcionario() {
    FILE *f = fopen(ARQ_FUNCIONARIOS, "rb");
    if (!f) return 1;
    Funcionario x;
    int max = 0;
    while (fread(&x, sizeof(Funcionario), 1, f) == 1) {
        if (x.codigo > max) max = x.codigo;
    }
    fclose(f);
    return max + 1;
}
int gerar_codigo_estadia() {
    FILE *f = fopen(ARQ_ESTADIAS, "rb");
    if (!f) return 1;
    Estadia e;
    int max = 0;
    while (fread(&e, sizeof(Estadia), 1, f) == 1) {
        if (e.codigo > max) max = e.codigo;
    }
    fclose(f);
    return max + 1;
}

//funcoes de Cliente

void cadastrar_cliente() {
    Cliente c;
    c.codigo = gerar_codigo_cliente();
    printf("Codigo gerado para cliente: %d\n", c.codigo);
    printf("Nome: ");
    limpar_buffer_scanf(); // Limpeza de buffer
    fgets(c.nome, sizeof(c.nome), stdin); trim_newline(c.nome);
    printf("Endereco: ");
    fgets(c.endereco, sizeof(c.endereco), stdin); trim_newline(c.endereco);
    printf("Telefone: ");
    fgets(c.telefone, sizeof(c.telefone), stdin); trim_newline(c.telefone);

    FILE *f = fopen(ARQ_CLIENTES, "ab");
    if (!f) { perror("Erro ao abrir arquivo de clientes"); return; }
    fwrite(&c, sizeof(Cliente), 1, f);
    fclose(f);
    printf("Cliente cadastrado com sucesso!\n");
}

void mostrar_cliente(const Cliente *c) {
    printf("Codigo: %d\nNome: %s\nEndereco: %s\nTelefone: %s\n",
           c->codigo, c->nome, c->endereco, c->telefone);
}

//busca por codigo ou nome (substring, case-sensitive simples
void pesquisar_cliente() {
    printf("Pesquisar cliente por (1) codigo ou (2) nome? ");
    int op; if (scanf("%d", &op) != 1) return;
    FILE *f = fopen(ARQ_CLIENTES, "rb");
    if (!f) { printf("Nenhum cliente cadastrado.\n"); return; }
    Cliente c;
    int achou = 0;
    if (op == 1) {
        int cod; printf("Codigo: "); scanf("%d", &cod);
        limpar_buffer_scanf(); // Limpa buffer após o scanf
        while (fread(&c, sizeof(Cliente), 1, f) == 1) {
            if (c.codigo == cod) { mostrar_cliente(&c); achou = 1; break; }
        }
    } else {
        char busca[80];
        printf("Nome (ou parte): ");
        limpar_buffer_scanf(); // Limpeza de buffer
        fgets(busca, sizeof(busca), stdin); trim_newline(busca);
        while (fread(&c, sizeof(Cliente), 1, f) == 1) {
            //busca simples: case-sensitive, aluno iniciante
            if (strstr(c.nome, busca) != NULL) { mostrar_cliente(&c); printf("----\n"); achou = 1; }
        }
    }
    if (!achou) printf("Cliente nao encontrado.\n");
    fclose(f);
}

//funcoes de Funcionario

void cadastrar_funcionario() {
    Funcionario func;
    func.codigo = gerar_codigo_funcionario();
    printf("Codigo gerado para funcionario: %d\n", func.codigo);
    printf("Nome: ");
    limpar_buffer_scanf(); // Limpeza de buffer
    fgets(func.nome, sizeof(func.nome), stdin); trim_newline(func.nome);
    printf("Telefone: ");
    fgets(func.telefone, sizeof(func.telefone), stdin); trim_newline(func.telefone);
    printf("Cargo: ");
    fgets(func.cargo, sizeof(func.cargo), stdin); trim_newline(func.cargo);
    printf("Salario: ");
    scanf("%f", &func.salario);
    limpar_buffer_scanf(); // Limpa buffer após o scanf

    FILE *f = fopen(ARQ_FUNCIONARIOS, "ab");
    if (!f) { perror("Erro ao abrir arquivo de funcionarios"); return; }
    fwrite(&func, sizeof(Funcionario), 1, f);
    fclose(f);
    printf("Funcionario cadastrado com sucesso!\n");
}

void mostrar_funcionario(const Funcionario *p) {
    printf("Codigo: %d\nNome: %s\nTelefone: %s\nCargo: %s\nSalario: %.2f\n",
           p->codigo, p->nome, p->telefone, p->cargo, p->salario);
}

void pesquisar_funcionario() {
    printf("Pesquisar funcionario por (1) codigo ou (2) nome? ");
    int op; if (scanf("%d", &op) != 1) return;
    FILE *f = fopen(ARQ_FUNCIONARIOS, "rb");
    if (!f) { printf("Nenhum funcionario cadastrado.\n"); return; }
    Funcionario p;
    int achou = 0;
    if (op == 1) {
        int cod; printf("Codigo: "); scanf("%d", &cod);
        limpar_buffer_scanf(); // Limpa buffer após o scanf
        while (fread(&p, sizeof(Funcionario), 1, f) == 1) {
            if (p.codigo == cod) { mostrar_funcionario(&p); achou = 1; break; }
        }
    } else {
        char busca[80];
        printf("Nome (ou parte): ");
        limpar_buffer_scanf(); // Limpeza de buffer
        fgets(busca, sizeof(busca), stdin); trim_newline(busca);
        while (fread(&p, sizeof(Funcionario), 1, f) == 1) {
            if (strstr(p.nome, busca) != NULL) { mostrar_funcionario(&p); printf("----\n"); achou = 1; }
        }
    }
    if (!achou) printf("Funcionario nao encontrado.\n");
    fclose(f);
}

//funcoes de Quarto

//verifica existencia de quarto por numero, se encontrado preenche q e retorna 1
int quarto_existe(int numero, Quarto *q_out) {
    FILE *f = fopen(ARQ_QUARTOS, "rb");
    if (!f) return 0;
    Quarto q;
    while (fread(&q, sizeof(Quarto), 1, f) == 1) {
        if (q.numero == numero) {
            if (q_out) *q_out = q;
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

// regrava todo arquivo de quartos substituindo o quarto com mesmo numero
int atualizar_quarto(Quarto q_atualizado) {
    FILE *f = fopen(ARQ_QUARTOS, "rb");
    if (!f) return 0;
    // le todos para memoria simples (contador)
    Quarto *lista = NULL;
    size_t cont = 0;
    Quarto temp;
    while (fread(&temp, sizeof(Quarto), 1, f) == 1) {
        lista = realloc(lista, (cont + 1) * sizeof(Quarto));
        lista[cont++] = temp;
    }
    fclose(f);
    // atualizar
    int achou = 0;
    for (size_t i = 0; i < cont; ++i) {
        if (lista[i].numero == q_atualizado.numero) {

            lista[i] = q_atualizado;
            achou = 1;
            break;
        }
    }
    // reescrever arquivo
    FILE *fw = fopen(ARQ_QUARTOS, "wb");
    if (!fw) { free(lista); return 0; }
    for (size_t i = 0; i < cont; ++i) fwrite(&lista[i], sizeof(Quarto), 1, fw);
    fclose(fw);
    free(lista);
    return achou;
}

void cadastrar_quarto() {
    Quarto q;
    printf("Numero do quarto (inteiro): "); scanf("%d", &q.numero);
    if (quarto_existe(q.numero, NULL)) {
        printf("Erro: quarto ja existe.\n"); return;
    }
    printf("Quantidade de hospedes: "); scanf("%d", &q.qtdHospedes);
    printf("Valor da diaria (R$): "); scanf("%f", &q.valorDiaria);

    limpar_buffer_scanf(); // Limpa o buffer após o último scanf

    q.ocupado = 0;
    FILE *f = fopen(ARQ_QUARTOS, "ab");
    if (!f) { perror("Erro ao abrir arquivo quartos"); return; }
    fwrite(&q, sizeof(Quarto), 1, f);
    fclose(f);
    printf("Quarto cadastrado com sucesso!\n");
}

//funcoes de Estadia

// verifica se existe alguma estadia ativa no mesmo quarto que conflita com periodo novo
int periodo_livre(int numeroQuarto, const char *entrada, const char *saida) {
    FILE *f = fopen(ARQ_ESTADIAS, "rb");
    if (!f) return 1; // sem estadias -> livre
    Estadia e;
    while (fread(&e, sizeof(Estadia), 1, f) == 1) {
        if (e.numeroQuarto == numeroQuarto && e.ativo == 1) {
            if (sobrepoe(entrada, saida, e.dataEntrada, e.dataSaida)) {
                fclose(f);
                return 0; // nao livre
            }
        }
    }
    fclose(f);
    return 1;
}

// regrava estadia por codigo (finalizar/atualizar)
int atualizar_estadia(Estadia e_atualizada) {
    FILE *f = fopen(ARQ_ESTADIAS, "rb");
    if (!f) return 0;
    Estadia *lista = NULL; size_t cont = 0; Estadia tmp;
    while (fread(&tmp, sizeof(Estadia), 1, f) == 1) {
        lista = realloc(lista, (cont + 1) * sizeof(Estadia));
        lista[cont++] = tmp;
    }
    fclose(f);
    int achou = 0;
    for (size_t i = 0; i < cont; ++i) {
        if (lista[i].codigo == e_atualizada.codigo) {
            lista[i] = e_atualizada;
            achou = 1; break;
        }
    }
    FILE *fw = fopen(ARQ_ESTADIAS, "wb");
    if (!fw) { free(lista); return 0; }
    for (size_t i = 0; i < cont; ++i) fwrite(&lista[i], sizeof(Estadia), 1, fw);
    fclose(fw); free(lista);
    return achou;
}

void cadastrar_estadia() {
    Estadia e;
    e.codigo = gerar_codigo_estadia();
    printf("Codigo gerado para estadia: %d\n", e.codigo);

    printf("Codigo do cliente: ");
    int temp_cod;
    if (scanf("%d", &temp_cod) != 1) {
        printf("Codigo do cliente invalido.\n");
        limpar_buffer_scanf();
        return;
    }
    e.codCliente = temp_cod;

    // verificar cliente existe
    FILE *fc = fopen(ARQ_CLIENTES, "rb");
    if (!fc) { printf("Nenhum cliente cadastrado.\n"); return; }
    Cliente tmpc; int cliente_ok = 0;
    while (fread(&tmpc, sizeof(Cliente), 1, fc) == 1) {
        if (tmpc.codigo == e.codCliente) { cliente_ok = 1; break; }
    }
    fclose(fc);
    if (!cliente_ok) { printf("Cliente nao encontrado.\n"); limpar_buffer_scanf(); return; }

    printf("Quantidade de hospedes: ");
    int qtd;
    if (scanf("%d", &qtd) != 1) {
        printf("Quantidade de hospedes invalida.\n");
        limpar_buffer_scanf();
        return;
    }
    limpar_buffer_scanf(); // Limpa o buffer APÓS o scanf de qtd

    // NOVO: Leitura de data separada
    if (!ler_e_formatar_data("Data de entrada", e.dataEntrada)) return;
    if (!ler_e_formatar_data("Data de saida", e.dataSaida)) return;

    int dias = diff_days_simple(e.dataEntrada, e.dataSaida);
    if (dias <= 0) {
        printf("Periodo invalido (saida deve ser apos entrada).\n");
        return;
    }
    e.qtdDiarias = dias;
    e.ativo = 1;

    // procurar quarto disponivel (desocupado e com capacidade >= qtd e sem periodo conflito)
    FILE *fq = fopen(ARQ_QUARTOS, "rb");
    if (!fq) { printf("Nenhum quarto cadastrado.\n"); return; }
    Quarto qtmp; int encontrado = 0;
    while (fread(&qtmp, sizeof(Quarto), 1, fq) == 1) {
        // Para simplificar, estamos considerando quartos "desocupados" (ocupado == 0) ou que não têm conflito de datas.
        // A lógica mais correta seria apenas verificar o conflito de datas, ignorando o campo 'ocupado'
        // que deve ser usado apenas para estadias sem data de saída (check-in atual), o que não é o caso aqui.
        if (qtmp.qtdHospedes >= qtd) {
            if (periodo_livre(qtmp.numero, e.dataEntrada, e.dataSaida)) {
                encontrado = 1;
                break;
            }
        }
    }
    fclose(fq);
    if (!encontrado) { printf("Nenhum quarto disponivel para o periodo e capacidade.\n"); return; }
    e.numeroQuarto = qtmp.numero;

    // gravar estadia
    FILE *fe = fopen(ARQ_ESTADIAS, "ab");
    if (!fe) { perror("Erro ao abrir arquivo estadias"); return; }
    fwrite(&e, sizeof(Estadia), 1, fe);
    fclose(fe);

    // Na lógica ideal, o quarto só ficaria ocupado se fosse uma estadia aberta,
    // mas mantemos a lógica original para evitar mudar as regras do seu trabalho.
    if (qtmp.ocupado == 0) {
        qtmp.ocupado = 1;
        if (!atualizar_quarto(qtmp)) {
            printf("Aviso: nao foi possivel atualizar status do quarto (arquivo quartos).\n");
        }
    }

    printf("Estadia cadastrada! Codigo: %d | Quarto: %d | Diarias: %d\n",
           e.codigo, e.numeroQuarto, e.qtdDiarias);
}

// dar baixa em uma estadia: definir ativo=0 e liberar quarto
void dar_baixa_estadia() {
    printf("Codigo da estadia para dar baixa: ");
    int cod; scanf("%d", &cod);
    limpar_buffer_scanf(); // Limpeza de buffer

    FILE *f = fopen(ARQ_ESTADIAS, "rb");
    if (!f) { printf("Nenhuma estadia registrada.\n"); return; }
    Estadia e; int achou = 0;
    while (fread(&e, sizeof(Estadia), 1, f) == 1) {
        if (e.codigo == cod) { achou = 1; break; }
    }
    fclose(f);
    if (!achou) { printf("Estadia nao encontrada.\n"); return; }
    if (e.ativo == 0) { printf("Estadia ja finalizada.\n"); return; }

    // obter valor diaria do quart
    Quarto q; if (!quarto_existe(e.numeroQuarto, &q)) { printf("Quarto nao encontrado (erro de consistencia).\n"); return; }
    float total = e.qtdDiarias * q.valorDiaria;
    printf("Total a pagar: %d diarias x R$ %.2f = R$ %.2f\n", e.qtdDiarias, q.valorDiaria, total);

    // marcar estadia como finalizada e atualizar arquivo
    e.ativo = 0;
    if (!atualizar_estadia(e)) { printf("Erro ao atualizar arquivo de estadias.\n"); return; }

    // liberar quarto
    q.ocupado = 0;
    if (!atualizar_quarto(q)) {
        printf("Erro ao atualizar arquivo de quartos.\n");
    } else {
        printf("Baixa registrada e quarto liberado.\n");
    }
}

// listar todas as estadias de um cliente (por codigo ou pelo nome)
void listar_estadias_cliente() {
    printf("Pesquisar estadias por (1) codigo ou (2) nome do cliente? ");
    int op; if (scanf("%d", &op) != 1) return;
    int cod = -1;
    char nomeBusca[80] = "";
    if (op == 1) {
        printf("Codigo do cliente: "); scanf("%d", &cod);
        limpar_buffer_scanf(); // Limpeza de buffer
    } else {
        printf("Nome (ou parte): ");
        limpar_buffer_scanf(); // Limpeza de buffer
        fgets(nomeBusca, sizeof(nomeBusca), stdin); trim_newline(nomeBusca);
    }

    FILE *fe = fopen(ARQ_ESTADIAS, "rb");
    if (!fe) { printf("Nenhuma estadia registrada.\n"); return; }
    FILE *fc = fopen(ARQ_CLIENTES, "rb");
    if (!fc) { printf("Nenhum cliente cadastrado.\n"); fclose(fe); return; }

    Estadia e; int achou = 0;
    while (fread(&e, sizeof(Estadia), 1, fe) == 1) {
        int mostrar = 0;
        if (op == 1) {
            if (e.codCliente == cod) mostrar = 1;
        } else {
            //achar nome do cliente (busca simples case-sensitive)
            rewind(fc);
            Cliente ctmp;
            while (fread(&ctmp, sizeof(Cliente), 1, fc) == 1) {
                if (ctmp.codigo == e.codCliente) {
                    if (strstr(ctmp.nome, nomeBusca) != NULL) mostrar = 1;
                    break;
                }
            }
        }
        if (mostrar) {
            printf("Estadia %d | Cliente %d | Quarto %d | %s -> %s | Diarias: %d | %s\n",
                   e.codigo, e.codCliente, e.numeroQuarto, e.dataEntrada, e.dataSaida, e.qtdDiarias,
                   e.ativo ? "ativa" : "finalizada");
            achou = 1;
        }
    }
    if (!achou) printf("Nenhuma estadia encontrada para o cliente.\n");
    fclose(fe); fclose(fc);
}

// calcular pontos de fidelidade: 10 pontos por diaria em todas as estadias do cliente
void calcular_pontos() {
    printf("Digite o codigo do cliente: ");
    int cod; scanf("%d", &cod);
    limpar_buffer_scanf(); // Limpeza de buffer

    FILE *fe = fopen(ARQ_ESTADIAS, "rb");
    if (!fe) { printf("Nenhuma estadia registrada.\n"); return; }
    Estadia e; int totalDiarias = 0;
    // CORRIGIDO: Agora usa 'fe'
    while (fread(&e, sizeof(Estadia), 1, fe) == 1) {
        if (e.codCliente == cod) totalDiarias += e.qtdDiarias;
    }
    fclose(fe);
    int pontos = totalDiarias * 10;
    printf("Cliente %d: %d diarias acumuladas -> %d pontos de fidelidade\n", cod, totalDiarias, pontos);
}

//funcoes auxiliares de listagem (debug/ajuda)

void listar_todos_clientes() {
    FILE *f = fopen(ARQ_CLIENTES, "rb");
    if (!f) { printf("Nenhum cliente cadastrado.\n"); return; }
    Cliente c;
    while (fread(&c, sizeof(Cliente), 1, f) == 1) {
        mostrar_cliente(&c); printf("----\n");
    }
    fclose(f);
}

void listar_todos_quartos() {
    FILE *f = fopen(ARQ_QUARTOS, "rb");
    if (!f) { printf("Nenhum quarto cadastrado.\n"); return; }
    Quarto q;
    while (fread(&q, sizeof(Quarto), 1, f) == 1) {
        printf("Quarto %d | Capacidade: %d | Valor: R$ %.2f | Status: %s\n",
               q.numero, q.qtdHospedes, q.valorDiaria, q.ocupado ? "ocupado" : "desocupado");
    }
    fclose(f);
}

void listar_todas_estadias() {
    FILE *f = fopen(ARQ_ESTADIAS, "rb");
    if (!f) { printf("Nenhuma estadia cadastrada.\n"); return; }
    Estadia e;
    while (fread(&e, sizeof(Estadia), 1, f) == 1) {
        printf("Estadia %d | Cliente %d | Quarto %d | %s -> %s | Diarias: %d | %s\n",
               e.codigo, e.codCliente, e.numeroQuarto, e.dataEntrada, e.dataSaida, e.qtdDiarias,
               e.ativo ? "ativa" : "finalizada");
    }
    fclose(f);
}

//menu e main

void menu() {
    printf("\n=== HOTEL DESCANSO GARANTIDO ===\n");
    printf("1 - Cadastrar cliente\n");
    printf("2 - Cadastrar funcionario\n");
    printf("3 - Cadastrar quarto\n");
    printf("4 - Cadastrar estadia\n");
    printf("5 - Dar baixa em estadia\n");
    printf("6 - Pesquisar cliente\n");
    printf("7 - Pesquisar funcionario\n");
    printf("8 - Ver estadias de um cliente\n");
    printf("9 - Calcular pontos de fidelidade\n");
    printf("10 - Listar todos clientes\n");
    printf("11 - Listar todos quartos\n");
    printf("12 - Listar todas estadias\n");
    printf("0 - Sair\n");
    printf("Escolha: ");
}

int main(void) {
    int opc;
    do {
        menu();
        if (scanf("%d", &opc) != 1) {
            limpar_buffer_scanf(); // Limpa buffer em caso de erro no menu
            printf("Entrada invalida. Saindo.\n"); break;
        }

        switch (opc) {
            case 1: cadastrar_cliente(); break;
            case 2: cadastrar_funcionario(); break;
            case 3: cadastrar_quarto(); break;
            case 4: cadastrar_estadia(); break;
            case 5: dar_baixa_estadia(); break;
            case 6: pesquisar_cliente(); break;
            case 7: pesquisar_funcionario(); break;
            case 8: listar_estadias_cliente(); break;
            case 9: calcular_pontos(); break;
            case 10: listar_todos_clientes(); break;
            case 11: listar_todos_quartos(); break;
            case 12: listar_todas_estadias(); break;
            case 0: printf("Tchau! Saindo...\n"); break;
            default: printf("Opcao invalida.\n"); break;
        }
    } while (opc != 0);
    return 0;
}
