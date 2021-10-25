#include<stdio.h>

unsigned char memoria[4096 * 4];  // Memoria inicializada com seu Tamanho
int registrador[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0,6144, 16380, 0, 0
                      }; // Registrados de $0 á $31
int HI = 0;              // Registrador HI
int LO = 0;              // Registrador LO
unsigned int op;         // Campo do Opcode
unsigned int rs;         // Campo do Registrador rs
unsigned int rd;         // Campo do Registrador rd
unsigned int rt;         // Campo do Registrador rt
unsigned int shamt;      // Campo do SHIft Amount
unsigned int funct;      // Campo da Função
int immediate;           // Campo da Constante Númerica
int adress;              // Variavel auxiliadora para salto no tipo J
int tipo;                // Variavel auxiliadora para definir o tipo de instruções
int End = 0;             // Variavel auxiliadora usada para finalizar o programa

/***** Funções utilizadas no programa *****/
unsigned int buscarInstrucao(int PC);                                // Função para buscar instrução
void decodificarInstrucao(unsigned int retorno);                     // Função para decodificar instrução
int tipoInstrucao(unsigned int opcode);                              // Função verifica qual formato de instrução
int executarInstrucao(int PC);                                       // Função para executar a instrução
int sinalEstendido(int immediate);                                   // Função para sinal estendido
int zeroEstendido(int immediate);                                    // Função para zero extendido
void imprimir(unsigned char memoria[4096 * 4],int registrador[32]);  // Função para imprimir os registradores e a memoria

int main(int argc, char * argv[])
{
    int i, j = 0;               // Varaiveis auxiliadoras
    int PC = 12288;             // Registrador PC inicializado
    unsigned int retorno;       // Variavel auxiliadora de retorno
    FILE * arquivo_text;        // Variavel para abrir o arquivo de texto
    FILE * arquivo_data;        // Variavel para abrir o arquivo de data

    arquivo_text = fopen(argv[1], "rb");  // Abertura de arquivo.text
    arquivo_data = fopen(argv[2], "rb");  // Abertura de arquivo.data

    if(arquivo_text == NULL || arquivo_data == NULL)
    {
        printf("Erro: não foi possível ler ou criar 1 ou mais arquivos\n");
    }
    else
    {
        if (argc == 3)
        {
            i = PC;
            while(fread(&memoria[i], sizeof(char), 1, arquivo_text) == 1)  // carrega as instruções do arquivo .text na memória
                i = i+1;
            while(fread(&memoria[j], sizeof(char), 1, arquivo_data) == 1)  // carrega os dados do arquivo .data na memória
                j = j+1;

            fclose(arquivo_text); // Fechamento do arquivo.text
            fclose(arquivo_data); // Fechamento do arquivo.data
            while (End == 0)
            {
                retorno = buscarInstrucao(PC);  // chamada da função para buscar a instrução apontado por PC e guarda em retorno
                PC = PC + 4;                    // Atualiza PC
                decodificarInstrucao(retorno);  // chamada da função para decodificar a instrução em retorno
                PC = executarInstrucao(PC);     // chamada da função para executar as instrução com base em seus campos
                if(End == 2)
                {
                    printf("Error: stack overfLOw.\n");
                    break;
                }
            }
            printf("\n");
            imprimir(memoria, registrador);     // chamada da função para imprimir a memoria e os registradores
        }
        else
            printf("Erro: quantidade de argumentos invalida!\n");
    }
    return 0;
}

unsigned int buscarInstrucao(int PC)
{
    unsigned int retorno = 0;    // Variavel auxiliadora de retorno

    // Manipulação de bits para juntar 4 bytes da instrução
    for(int posicao = PC + 3; posicao >= PC; posicao--)
    {
        retorno  = retorno | memoria[posicao];
        if(posicao != PC)
            retorno = retorno << 8;
    }
    return retorno;
}

void decodificarInstrucao(unsigned int retorno)
{
    // Extrai campos da instrução de retorno através da manipulação de bits
    op = (retorno & 0xfc000000) >> (32 - 6);
    rs = (retorno & 0x03e00000) >> (32 - 11);
    rt = (retorno & 0x001f0000) >> (32 - 16);
    rd = (retorno & 0x0000f800) >> (32 - 21);
    shamt = (retorno & 0x000007c0) >> (32 - 26);
    funct = (retorno & 0x0000003f) >> (32 - 32);
    immediate = (retorno & 0x0000ffff) >> (32 - 32);
    adress = (retorno & 0x003fffff) >> (32 - 32);
}

int tipoInstrucao(unsigned int opcode)
{
    int tipo = 0;
    /** Tipo R e Syscalls **/
    if(opcode == 0)
    {
        tipo = 0;
    }
    /** Tipo I **/
    else if(opcode >= 4 && opcode <= 43)
    {
        tipo = 1;
    }
    /** Tipo j **/
    else if (opcode == 2 || opcode == 3)
    {
        tipo = 2;
    }
    return tipo;
}

int sinalEstendido(int immediate)
{
    if ((immediate & 0x00008000) != 0)
        immediate = (immediate | 0xffff0000);
    return immediate;
}

int zeroEstendido(int immediate)
{
    immediate = (immediate & 0x0000ffff);
    return immediate;
}

int executarInstrucao(int PC)
{
    int                 endereco;   // Variavel auxiliadora para guardar endereço
    int                 i;          // Variavel auxiliadora
    unsigned long int 	aux;        // Variavel auxiliadora para instrução mult

    tipo = tipoInstrucao(op);       // Chamada da função para saber qual tipo de instrução será
    /************ Introções do tipo R e Syscalls ************/
    if (tipo == 0)
    {
        switch (funct)
        {
        case 0:     /** Instrução sll **/
            registrador[rd] = registrador[rt] << shamt;
            break;

        case 2:     /** Instrução srl **/
            registrador[rd] = registrador[rt] >> shamt;
            break;

        case 3:     /** Instrução sra **/
            registrador[rd] = registrador[rt] >> sinalEstendido(shamt);
            break;

        case 8:     /** Instrução jr **/
            PC = registrador[rs];
            break;

        case 12:    /** syscalls **/
            if(registrador[2] == 1)     /** syscall 1 imprimir inteiro **/
                printf("%d", registrador[4]);
            else if (registrador[2] == 4)      /** syscall 4 imprimir string **/
            {
                i = 0;
                while((memoria[registrador[4]+i]) != '\0')
                {
                    printf("%c", memoria[registrador[4]+i]);
                    i++;
                }
            }
            else if (registrador[2] == 5)   /** syscall 5 le inteiro **/
            {
                scanf("%d", &registrador[2]);
            }
            else if (registrador[2] == 8)   /** syscall 8 le string **/
            {
                fgets(((char *)&(memoria[registrador[4]])), registrador[5], stdin);
            }
            else if (registrador[2] == 10)  /** syscall 10, sai do programa **/
            {
                End = 1;   // vaLOr de retorno para finalizar programa
            }
            else if (registrador[2] == 12)   /** syscall 12 le um char **/
            {
                scanf("%c", (char*)&registrador[2]);
            }
            else if (registrador[2] == 34)   /** syscall 34 imprimir em hexadecimal **/
            {
                printf("0x%.8x", registrador[4]);
            }
            else if (registrador[2] == 36)   /** syscall 35 imprimir um inteiro sem sinal **/
            {
                if(registrador[2] < 0)
                {
                    registrador[2] = registrador[2] * -1;
                }
                printf("%d", registrador[4]);
            }
            else
            {
                printf("Syscall não implementado\n");
            }
            break;

        case 16:    /** Instrução mfHI **/
            registrador[rd] = HI;
            break;

        case 18:    /** Instrução mfLO **/
            registrador[rd] = LO;
            break;

        case 24:    /** Instrução mult **/
            aux = registrador[rs] * registrador[rt];
            HI = ((aux & 0xffffffff00000000) >> 32);
            LO = (aux & 0x00000000ffffffff);
            break;

        case 26:    /** Instrução div **/
            LO = registrador[rs] / registrador[rt];
            HI = registrador[rs] % registrador[rt];
            break;

        case 32:    /** Instrução add **/
            registrador[rd] = registrador[rs] + registrador[rt];
            break;

        case 34:    /** Instrução sub **/
            registrador[rd] = registrador[rs] - registrador[rt];
            break;

        case 36:    /** Instrução and **/
            registrador[rd] = registrador[rs] & registrador[rt];
            break;

        case 37:    /** Instrução or **/
            registrador[rd] = registrador[rs] | registrador[rt];
            break;

        case 42:    /** Instrução slt **/
            if (registrador[rs] < registrador[rt])
                registrador[rd] = 1;
            else
                registrador[rd] = 0;
            break;
        }
    }
    /************ Introções do tipo I ************/
    if(tipo == 1)
    {
        switch(op)
        {
        case 4:     /** Instrução beq **/
            if (registrador[rs] == registrador[rt])
                PC = PC + (sinalEstendido(immediate) << 2);
            break;

        case 5:     /** Instrução bne **/
            if (registrador[rs] != registrador[rt])
                PC = PC + (sinalEstendido(immediate) << 2);
            break;

        case 8:     /** Instrução addi **/
            registrador[rt] = registrador[rs] + sinalEstendido(immediate);
            break;

        case 9:     /** Instrução addiu **/
            registrador[rt] = registrador[rs] + zeroEstendido(immediate);
            break;

        case 10:    /** Instrução slti **/
            if (registrador[rs] < immediate)
                registrador[rt] = 1;
            else
                registrador[rt] = 0;
            break;

        case 12:    /** Instrução andi **/
            registrador[rt] = registrador[rs] & zeroEstendido(immediate);
            break;

        case 13:    /** Instrução ori **/
            registrador[rt] = registrador[rs] | zeroEstendido(immediate);
            break;

        case 15:    /** Instrução lui **/
            registrador[rt] = immediate << 31;
            break;

        case 32:    /** Instrução lb **/
            endereco = registrador[rs] + sinalEstendido(immediate);
            registrador[rt] = memoria[endereco];
            break;

        case 33:    /** Instrução lh **/
            endereco = registrador[rs] + sinalEstendido(immediate);
            registrador[rt] = memoria[endereco];
            registrador[rt] += (memoria[endereco+1] << 8);
            break;

        case 35:    /** Instrução lw **/
            endereco = registrador[rs] + sinalEstendido(immediate);
            registrador[rt] = (memoria[endereco+3] << 8);
            registrador[rt] += (memoria[endereco+2] << 8);
            registrador[rt] += (memoria[endereco+1] << 8);
            registrador[rt] += (memoria[endereco]);
            break;

        case 40:    /** Instrução sb **/
            endereco = registrador[rs] + sinalEstendido(immediate);
            memoria[endereco] = (registrador[rt] & 0x000000ff);
            break;

        case 41:    /** Instrução sh **/
            endereco = registrador[rs] + sinalEstendido(immediate);
            memoria[endereco] = (registrador[rt] & 0x000000ff);
            memoria[endereco+1] = (registrador[rt] & 0x0000ff00) >> 8;
            break;

        case 43:    /** Instrução sw **/
            endereco = registrador[rs] + sinalEstendido(immediate);
            memoria[endereco] = (registrador[rt] & 0x000000ff);
            memoria[endereco+1] = (registrador[rt] & 0x0000ff00) >> 8;
            memoria[endereco+2] = (registrador[rt] & 0x00ff0000) >> 16;
            memoria[endereco+3] = (registrador[rt] & 0xff000000) >> 24;
            break;
        }
    }

    /************ Introções do tipo J ************/
    if(tipo == 2)
    {
        switch(op)
        {
        case 2:     /** Instrução j **/
            PC = ((PC) & 0xf0000000) | (adress << 2);
            break;

        case 3:     /** Instrução jal **/
            registrador[31] = PC;
            PC = ((PC) & 0xf0000000) | (adress << 2);
            if(PC <= 12288)
            {
                End = 2;
            }
            break;
        }
    }

    return PC;

}

void imprimir(unsigned char memoria[4096 * 4],int registrador[32])
{
    int i;  // Variavel auxiliadora

    for (i = 0; i < 32; ++i)
    {
        printf("$%d 0x%.8x\n", i, registrador[i]);
    }
    for (i = 0; i < 4096 * 4; i = i + 4)
    {
        printf("Mem[0x%.8x] 0x%.8x 0x%.8x 0x%.8x 0x%.8x\n", i, memoria[i], memoria[i+1], memoria[i+2], memoria[i+3]);
    }
}

