// ----------------------------------------------------------------------------------------------------
//    BIBLIOTECAS
// ----------------------------------------------------------------------------------------------------

#include <iostream>     // entrada e saida
#include <fstream>      // leitura e escrita de arquivos
#include <vector>       // vetores
using namespace std;

// ----------------------------------------------------------------------------------------------------
//    MEMORIA
// ----------------------------------------------------------------------------------------------------

enum OPCODE {ADD=1, SUB, MULT, DIV, JMP, JMPN, JMPP, JMPZ, COPY, LOAD, STORE, INPUT, OUTPUT, STOP};
vector<int> MEM;    // lista de instrucoes do programa em execucao
int ACC = 0;        // registrador unico: acumulador
int PC;             // contador de programa

// ----------------------------------------------------------------------------------------------------
//    SIMULADOR
// ----------------------------------------------------------------------------------------------------

// valida numero inteiro inserido pelo usuario
void insert (int* IN) {
    string INAUX;

    std::cin >> INAUX;
    while (!((INAUX.front() >= 48) && (INAUX.front() <= 57))) {
        std::cout << "ERRO! Insira um inteiro valido: ";
        std::cin >> INAUX;
    }
    *IN = stoi(INAUX);
}

// executa o programa
void run () {
    int OP1, OP2, IN;   // variaveis auxiliares: operandos e valor de entrada

    // enquanto programa nao acabou, pega instrucao
    for (PC=0; PC < MEM.size(); PC++) {
        switch ( MEM[PC] ) {

            // operacoes aritmeticas
            case ADD:                   // se adicao
                OP1 = MEM[++PC];        // pega valor no endereco de memoria apontado pelo operando
                ACC = ACC + MEM[ OP1];  // soma conteudo do acumulador com valor e guarda
                std::cout << "ACC: " << ACC << endl;
                break;
            case SUB:                   // se subtracao
                OP1 = MEM[++PC];        // pega valor no endereco de memoria apontado pelo operando
                ACC = ACC - MEM[ OP1];  // subtrai valor do conteudo do acumulador e guarda
                std::cout << "ACC: " << ACC << endl;
                break;
            case MULT:                  // se multiplicacao
                OP1 = MEM[++PC];        // pega valor no endereco de memoria apontado pelo operando
                ACC = ACC * MEM[ OP1];  // multiplica conteudo do acumulador com valor e guarda
                std::cout << "ACC: " << ACC << endl;
                break;
            case DIV:                   // se divisao
                OP1 = MEM[++PC];        // pega valor no endereco de memoria apontado pelo operando
                ACC = ACC / MEM[ OP1];  // divide conteudo do acumulador pelo valor e guarda
                std::cout << "ACC: " << ACC << endl;
                break;

            // saltos
            case JMP:               // se salto incondicional
                OP1 = MEM[++PC];    // pega endereco de memoria apontado pelo operando
                PC = --OP1;         // contador de programa recebe endereco
                std::cout << "PC: " << PC << endl;
                break;
            case JMPN:              // se salto condicional
                OP1 = MEM[++PC];    // pega endereco de memoria apontado pelo operando
                if (ACC < 0) {      // se acumulador for numero negativo
                    PC = --OP1;     // contador de programa recebe endereco
                    std::cout << "PC: " << PC << endl;
                } break;
            case JMPP:              // se salto condicional
                OP1 = MEM[++PC];    // pega endereco de memoria apontado pelo operando
                if (ACC > 0) {      // se acumulador for numero positivo
                    PC = --OP1;     // contador de programa recebe endereco
                    std::cout << "PC: " << PC << endl;
                } break;
            case JMPZ:              // se salto condicional
                OP1 = MEM[++PC];    // pega endereco de memoria apontado pelo operando
                if (ACC == 0) {     // se acumulador for igual a zero
                    PC = --OP1;     // contador de programa recebe endereco
                    std::cout << "PC: " << PC << endl;
                } break;

            // acesso a memoria
            case COPY:                  // se copia
                OP1 = MEM[++PC];        // pega valor no endereco de memoria apontado pelo operando1
                OP2 = MEM[++PC];        // pega endereco de memoria apontado pelo operando2
                MEM[ OP2] = MEM[ OP1];  // guarda valor no endereco de memoria
                std::cout << OP2 << ": " << MEM[ OP2] << endl;
                break;
            case LOAD:              // se leitura de memoria
                OP1 = MEM[++PC];    // pega valor no endereco de memoria apontado pelo operando
                ACC = MEM[ OP1];    // guarda valor no acumulador
                std::cout << "ACC: " << ACC << endl;
                break;
            case STORE:             // se escrita em memoria
                OP1 = MEM[++PC];    // pega endereco de memoria apontado pelo operando
                MEM[ OP1] = ACC;    // guarda conteudo do acumulador no endereco de memoria
                std::cout << OP1 << ": " << MEM[ OP1] << endl;
                break;

            // entrada e saida
            case INPUT:             // se entrada
                OP1 = MEM[++PC];    // pega endereco de memoria apontado pelo operando
                std::cout << "Favor, inserir um valor numerico: ";
                insert(&IN);        // recebe valor inserido pelo usuario
                MEM[ OP1] = IN;     // guarda valor no endereco de memoria
                std::cout << OP1 << ": " << MEM[ OP1] << endl;
                break;
            case OUTPUT:            // se saida
                OP1 = MEM[++PC];    // mostra valor guardado no endereco apontado pelo operando
                std::cout << "O valor de saida eh: " << MEM[ OP1] << endl;
                break;

            // fim
            case STOP:      // se parada
                std::cout << "Fim do programa." << endl;
                return;     // finaliza programa
            default: break;
        }
    }
}

// inicio
int main (int argc, char *argv[]) {
    
    // recebe nome de arquivo
    string file_name;
    if (argc > 1) {
        file_name = argv[1];
    } else {
        std::cout << "Insira o nome do arquivo: " ;
        std::cin >> file_name;
    }
    // abre arquivo contendo o codigo objeto
    std::cout << endl << "Inicio do programa" << endl;
    ifstream file (file_name);
    int current;

    // le todo o arquivo .obj
    if ( file.is_open() ) {
        while (file >> current)
            MEM.push_back(current);
        file.close();
    } else
        std::cout << endl << "ERRO! Arquivo nao encontrado." << endl;

    // executa o codigo
    run();

    return 0;
}
