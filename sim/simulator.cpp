// ----------------------------------------------------------------------------------------------------
//    BIBLIOTECAS
// ----------------------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;

// ----------------------------------------------------------------------------------------------------
//    MEMORIA
// ----------------------------------------------------------------------------------------------------

enum e_OPCODE {ADD=1, SUB, MULT, DIV, JMP, JMPN, JMPP, JMPZ, COPY, LOAD, STORE, INPUT, OUTPUT, STOP};
vector<int> MEM;
int ACC = 0;
int PC;

// ----------------------------------------------------------------------------------------------------
//    SIMULADOR
// ----------------------------------------------------------------------------------------------------

void run () {
    int op1, op2, in;

    for (PC=0; PC < MEM.size(); PC++) {
        switch ( MEM[PC] ) {
            case ADD:
                std::cout << "ADD " << MEM[PC+1] << "\t";
                ACC = ACC + MEM[ MEM[++PC] ];
                std::cout << "ACC = " << ACC << endl;
                break;
            case SUB:
                std::cout << "SUB " << MEM[PC+1] << "\t";
                ACC = ACC - MEM[ MEM[++PC] ];
                std::cout << "ACC = " << ACC << endl;
                break;
            case MULT:
                std::cout << "MULT " << MEM[PC+1] << "\t";
                ACC = ACC * MEM[ MEM[++PC] ];
                std::cout << "ACC = " << ACC << endl;
                break;
            case DIV:
                std::cout << "DIV " << MEM[PC+1] << "\t";
                ACC = ACC / MEM[ MEM[++PC] ];
                std::cout << "ACC = " << ACC << endl;
                break;
            case JMP:
                std::cout << "JMP " << MEM[PC+1] << endl;
                PC = MEM[++PC];
                std::cout << "PC = " << PC << endl;
                PC--;
                break;
            case JMPN:
                std::cout << "JMPN " << MEM[PC+1] << endl;
                if (ACC < 0) {
                    PC = MEM[++PC];
                    PC--;
                } else PC++;
                break;
            case JMPP:
                std::cout << "JMPP " << MEM[PC+1] << "\tPC = " << PC << endl;
                if (ACC > 0) {
                    PC = MEM[++PC];
                    std::cout << "PC = " << PC << endl;
                    PC--;
                } else PC++;
                break;
            case JMPZ:
                std::cout << "JMPZ " << MEM[PC+1] << endl;
                if (ACC == 0) {
                    PC = MEM[++PC];
                    PC--;
                } else PC++;
                break;
            case COPY:
                std::cout << "COPY " << MEM[PC+1] << "," << MEM[PC+2] << endl;
                op1 = MEM[++PC];
                op2 = MEM[++PC];
                MEM[op2] = MEM[op1];
                break;
            case LOAD:
                std::cout << "LOAD " << MEM[PC+1] << "\t";
                ACC = MEM[ MEM[++PC] ];
                std::cout << "ACC = " << ACC << endl;
                break;
            case STORE:
                std::cout << "STORE " << MEM[PC+1] << endl;
                MEM[ MEM[++PC] ] = ACC;
                break;
            case INPUT:
                std::cout << "Favor, inserir um valor numerico: ";
                std::cin >> in;
                MEM[ MEM[++PC] ] = in;
                break;
            case OUTPUT:
                std::cout << "O valor de saida eh: " << MEM[ MEM[++PC] ] << endl;
                break;
            case STOP:
                std::cout << "Fim do programa." << endl;
                return;
            default: break;
        }
    }
}

int main () {
    // abre arquivo contendo o codigo objeto
    string file_name = "bin.obj"; // nota: essa string depois vai ser o argumento iniciado junto ao programa na linha de comando
    ifstream file (file_name);
    int current;

    // le todo o arquivo .obj
    if ( file.is_open() ) {
        while (file >> current)
            MEM.push_back(current);
    } file.close();

    // executa o codigo
    run();

    return 0;
}
