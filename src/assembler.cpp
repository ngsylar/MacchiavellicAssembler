#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
using namespace std;

string line;        // guarda uma linha do codigo fonte
int position = 0;   // conta a posicao da linha no codigo fonte
int address = 0;    // conta a posicao de memoria do token

enum OPCODE {zero, ADD, SUB, MULT, DIV, JMP, JMPN, JMPP, JMPZ, COPY, LOAD, STORE, INPUT, OUTPUT, STOP};
enum DIRECTIVE {dzero, SECTION, SPACE, dCONST, EQU, dIF};
static map<string, OPCODE> mapOP;
static map<string, DIRECTIVE> mapDIR;

void stringSwitch () {
    mapOP["ADD"] = ADD;
    mapOP["SUB"] = SUB;
    mapOP["MULT"] = MULT;
    mapOP["DIV"] = DIV;
    mapOP["JMP"] = JMP;
    mapOP["JMPN"] = JMPN;
    mapOP["JMPP"] = JMPP;
    mapOP["JMPZ"] = JMPZ;
    mapOP["COPY"] = COPY;
    mapOP["LOAD"] = LOAD;
    mapOP["STORE"] = STORE;
    mapOP["INPUT"] = INPUT;
    mapOP["OUTPUT"] = OUTPUT;
    mapOP["STOP"] = STOP;
    mapDIR["SECTION"] = SECTION;
    mapDIR["SPACE"] = SPACE;
    mapDIR["CONST"] = dCONST;
    mapDIR["EQU"] = EQU;
    mapDIR["IF"] = dIF;
}

void decode () {
    istringstream tokenizer {line};
    string token;
    position++;

    while (tokenizer >> token) {
        // cout << token << endl;

        switch ( mapOP[token] ) {
            case ADD:       cout << token << endl;
                            break;
            case SUB:       cout << token << endl;
                            break;
            case MULT:      cout << token << endl;
                            break;
            case DIV:       cout << token << endl;
                            break;
            case JMP:       cout << token << endl;
                            break;
            case JMPN:      cout << token << endl;
                            break;
            case JMPP:      cout << token << endl;
                            break;
            case JMPZ:      cout << token << endl;
                            break;
            case COPY:      cout << token << endl;
                            break;
            case LOAD:      cout << token << endl;
                            break;
            case STORE:     cout << token << endl;
                            break;
            case INPUT:     cout << token << endl;
                            break;
            case OUTPUT:    cout << token << endl;
                            break;
            case STOP:      cout << token << endl;
                            break;
            default:
                if ( token[ token.size()-1 ] == ':') {
                    cout << address << ' ' << token << endl;
                    address++; // precisa ajustar para space
                }
                else if (token[0] == ';') {
                    // aqui o endereco nao pode contar
                    while (tokenizer >> token);
                }
                break;
        }
    }
}

int main () {
    stringSwitch();

    ifstream file ("bin.asm");
    if ( file.is_open() ) {
        while ( !file.eof() ) {
            getline (file, line);
            decode();
            // cout << line << endl;
        }
        file.close();
    }
    else cout << "fail";

    // cout << position << endl;
    return 0;
}
