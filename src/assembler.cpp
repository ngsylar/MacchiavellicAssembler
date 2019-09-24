#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
using namespace std;

string line;            // guarda uma linha do codigo fonte
int line_number = 0;    // conta a posicao da linha no codigo fonte
int address = 0;        // conta a posicao de memoria do token

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

void check_label (string* file_name, string label) {
    if ((label[0] >= 48) && (label[0] <= 57)) {
        cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
        cout << "lexicon error: label \"" << label << "\" starts with a number" << endl;
    }
    if (label.size() > 50) {
        cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
        cout << "lexicon error: label is longer than 50 characters:" << endl;
        cout << "\t\"" << label << "\"" << endl;
    }
    for (unsigned int i = 0; i < label.size(); i++) {
        if ((label[i] != 95) && (!((label[i] >= 48) && (label[i] <= 57)) && !((label[i] >= 65) && (label[i] <= 90)))) {
            cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
            cout << "lexicon error: label \"" << label << "\" is not just letters, numbers or \"_\" (underscore)" << endl;
            break;
        }
    }
}

void decode (string* file_name) {
    istringstream tokenizer {line};
    string token;
    line_number++;

    while (tokenizer >> token) {
        // cout << token << endl;

        switch ( mapOP[token] ) {
            case ADD:       // cout << token << endl;
                            break;
            case SUB:       // cout << token << endl;
                            break;
            case MULT:      // cout << token << endl;
                            break;
            case DIV:       // cout << token << endl;
                            break;
            case JMP:       // cout << token << endl;
                            break;
            case JMPN:      // cout << token << endl;
                            break;
            case JMPP:      // cout << token << endl;
                            break;
            case JMPZ:      // cout << token << endl;
                            break;
            case COPY:      // cout << token << endl;
                            break;
            case LOAD:      // cout << token << endl;
                            break;
            case STORE:     // cout << token << endl;
                            break;
            case INPUT:     // cout << token << endl;
                            break;
            case OUTPUT:    // cout << token << endl;
                            break;
            case STOP:      // cout << token << endl;
                            break;
            default:
                if ( token[ token.size()-1 ] == ':' ) {
                    // cout << address << ' ' << token << endl;
                    token.pop_back();
                    check_label(file_name, token);
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

    string file_name = "bin.asm"; // essa string depois vai ser o argumento iniciado junto ao programa na linha de comando

    ifstream file (file_name);    
    if ( file.is_open() ) {
        while ( !file.eof() ) {
            getline (file, line);
            for (auto & c: line) c = toupper(c);
            decode (&file_name);
            // cout << line << endl;
        }
        file.close();
    }
    else cout << "ERROR: File not found!";

    // cout << line_number << endl;
    return 0;
}
