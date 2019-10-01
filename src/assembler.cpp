#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
using namespace std;

string line;            // guarda uma linha do codigo fonte
int line_number = 0;    // conta a posicao da linha no codigo fonte
int address = 0;        // conta a posicao de memoria do token

enum e_OPCODE {ADD=1, SUB, MULT, DIV, JMP, JMPN, JMPP, JMPZ, COPY, LOAD, STORE, INPUT, OUTPUT, STOP};
enum e_DIRECTIVE {d_SECTION=1, d_SPACE, d_CONST, d_EQU, d_IF};
enum e_SECTION {s_TEXT=1, s_DATA};
static map<string, e_OPCODE> OPCODE;
static map<string, e_DIRECTIVE> DIRECTIVE;
static map<string, e_SECTION> SECTION;

// mapeia strings para serem usadas na estrutura condicional switch
void stringSwitch () {
    OPCODE["ADD"]       = ADD;
    OPCODE["SUB"]       = SUB;
    OPCODE["MULT"]      = MULT;
    OPCODE["DIV"]       = DIV;
    OPCODE["JMP"]       = JMP;
    OPCODE["JMPN"]      = JMPN;
    OPCODE["JMPP"]      = JMPP;
    OPCODE["JMPZ"]      = JMPZ;
    OPCODE["COPY"]      = COPY;
    OPCODE["LOAD"]      = LOAD;
    OPCODE["STORE"]     = STORE;
    OPCODE["INPUT"]     = INPUT;
    OPCODE["OUTPUT"]    = OUTPUT;
    OPCODE["STOP"]      = STOP;

    DIRECTIVE["SECTION"]    = d_SECTION;
    DIRECTIVE["SPACE"]      = d_SPACE;
    DIRECTIVE["CONST"]      = d_CONST;
    DIRECTIVE["EQU"]        = d_EQU;
    DIRECTIVE["IF"]         = d_IF;
    
    SECTION["TEXT"] = s_TEXT;
    SECTION["DATA"] = s_DATA;
}

// analise lexica: verifica validade dos rotulos inseridos no codigo fonte
void check_label (string* file_name, string label) {
    // se o primeiro caractere for um numero: erro
    if ((label[0] >= 48) && (label[0] <= 57)) {
        cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
        cout << "lexicon error: label \"" << label << "\" starts with a number" << endl;
    }
    // se o rotulo eh maior que 50 caracteres: erro
    if (label.size() > 50) {
        cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
        cout << "lexicon error: label is longer than 50 characters:" << endl;
        cout << "\t\"" << label << "\"" << endl;
    }
    // se o rotulo nao eh composto apenas por letras, numeros e underscore: erro
    for (unsigned int i = 0; i < label.size(); i++) {
        if ((label[i] != 95) && (!((label[i] >= 48) && (label[i] <= 57)) && !((label[i] >= 65) && (label[i] <= 90)))) {
            cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
            cout << "lexicon error: label \"" << label << "\" is not just letters, numbers or \"_\" (underscore)" << endl;
            break;
        }
    }
}

// passagem unica
void onepass (string* file_name) {
    istringstream tokenizer {line};
    string token;
    line_number++;

    while (tokenizer >> token) {
        // cout << token << endl;

        switch ( OPCODE[token] ) {
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

// pre-processamento
void preprocessing (string* file_name) {
    istringstream tokenizer {line};
    string token;
    line_number++;

    while (tokenizer >> token) {
        // cout << token << endl;

        switch ( DIRECTIVE[token] ) {
            case d_SECTION:
                tokenizer >> token;
                switch ( SECTION[token] ) {
                    case s_TEXT: break;
                    case s_DATA: break;
                } break;
            case d_SPACE:   // cout << token << endl;
                            break;
            case d_CONST:   // cout << token << endl;
                            break;
            case d_EQU:     // cout << token << endl;
                            break;
            case d_IF:      // cout << token << endl;
                            break;
            default:
                break;
        }
    }
}

int main () {
    stringSwitch();

    string file_name = "testing.asm"; // essa string depois vai ser o argumento iniciado junto ao programa na linha de comando

    ifstream file (file_name);
    if ( file.is_open() ) {
        while ( !file.eof() ) {
            getline (file, line);                   // le linha do codigo fonte
            for (auto & c: line) c = toupper(c);    // retira sensibilidade ao caso
            preprocessing (&file_name);             // realiza o pre-processamento
            // onepass (&file_name); // vai precisar entrar em outro laco fora deste, usando como arquivo fonte o codigo pre processado
            // cout << line << endl;
        }
        file.close();
    }
    else cout << endl << "ERROR: File not found!";

    // cout << line_number << endl;
    return 0;
}
