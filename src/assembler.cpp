#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <map>
using namespace std;

// criar class position: relacionar numero da linha do arquivo preprocessado com o arquivo fonte

static string line;             // guarda uma linha do codigo fonte ou pre-processado
static string symbol;           // auxiliar para guardar um token
static int line_number = 0;     // conta a posicao da linha no codigo fonte
static int address = 0;         // conta a posicao de memoria do token

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

// class Counter {
// };

// classe para relacionar tokens
class Link {
    public:
    string symbol;      // identificador ou rotulo
    string value;       // sinonimo ou simbolo

    // cria uma nova relacao
    void insert (string symbol, string value) {
        this->symbol = symbol;
        this->value = value;
    }
};
static Link relation;
static queue<Link> table;

class Analyze {
    public:
    string content;
    
    // conteudo recebe token
    void operator= (string token) {
        content = token;
    }

    // verifica ou exclui conteudo do objeto
    int empty ()    { return content.empty(); }
    void clear ()   { content.clear(); }

    // analise: verifica se ha mais de um rotulo na mesma linha
    int check_duplicate (istringstream* tokenizer, string* token) {
        string* label = &content;

        *tokenizer >> *token;                   // pega o proximo token
        if (token->back() == ':') {             // se token for rotulo entao
            do {
                token->pop_back();              // descarta ':'
                *label = *token;                // rotulo recebe token
                *tokenizer >> *token;           // pega proximo token
            } while (token->back() == ':');     // repetir laco enquanto token for rotulo
            return 1;                           // retorna 1 se houver mais de um rotulo
        } else
            return 0;                           // se nao, retorna 0
    }

    // analise: verifica validade dos rotulos inseridos no codigo fonte
    void check (string* file_name, istringstream* tokenizer, string* token) {
        string label = content;

        // se label for igual a uma instrucao ou diretiva
        if (((OPCODE[label] >= 1) && (OPCODE[label] <= 14)) || ((DIRECTIVE[label] >= 1) && (DIRECTIVE[label] <= 5)) || ((SECTION[label] >= 1) && (SECTION[label] <= 2))) {
            cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
            cout << "semantic error: invalid label (\"" << label << "\" is a reserved word)" << endl;
        }
        // se o primeiro caractere for um numero: erro
        if ((label.front() >= 48) && (label.front() <= 57)) {
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
                cout << "lexicon error: label \"" << label << "\" is not just letters, numbers or underscore" << endl;
                break;
            }
        }
        // verifica se ha mais de um rotulo na mesma linha
        if (check_duplicate (tokenizer, token)) {
            cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
            cout << "syntactic error: more than one label on the same line" << endl;
        }
    }
};

// passagem unica
void onepass (string* file_name) {
    istringstream tokenizer {line};
    string token;
    // line_number++;

    while (tokenizer >> token) {
        // cout << token << endl;

        switch ( DIRECTIVE[token] ) {
            case d_SECTION:
                tokenizer >> token;
                switch ( SECTION[token] ) {
                    case s_TEXT: break; // aqui devera vir o switch para opcodes
                    case s_DATA: break;
                } break;
            case d_SPACE:   // cout << token << endl;
                            break;
            case d_CONST:   // cout << token << endl;
                            break;
            default:
                break;
        }

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
                // se token for rotulo
                if ( token.back() == ':' ) {
                    // cout << address << ' ' << token << endl;
                    token.pop_back();
                    static Analyze label;
                    label = token;
                    label.check (file_name, &tokenizer, &token);
                    address++; // precisa ajustar para space
                }
                break;
        }
    }
}

// pre-processamento
void preprocessing (string* file_name) {
    Analyze ident;                  // identificador a ser analisado
    queue<Link> it;                 // iterador de Link
    istringstream tokenizer {line}; // decompositor de linha
    string token;                   // token
    line_number++;

    while (tokenizer >> token) {
        // cout << token << endl;

        // se houver rotulos repetidos, pular para o ultimo
        if (token.back() == ':') {
            token.pop_back();
            ident = token;
            ident.check_duplicate (&tokenizer, &token);
            // se houver quebra de linha apos rotulo, sai do laco (nao limpa a string token)
            if ( tokenizer.eof() || (token.front() == ';')) {
                symbol = ident.content;
            }
        } else if (!symbol.empty()) {
            ident = symbol;
            symbol.clear();
        }

        // analisa o token (o token seguinte ao rotulo, se existir rotulo)
        switch ( DIRECTIVE[ token ] ) {

            case d_EQU: // nota: lembrar que o rotulo pode vir em uma linha diferente
                // se identificador nao estiver vazio, substituir por valor constante
                if (!ident.empty()) {
                    ident.check (file_name, &tokenizer, &token);
                    relation.insert (ident.content, token);
                    table.push (relation);
                    // cout << ident.content << ": " << token << endl;
                    // cout << relation.symbol << ": " << relation.value << endl;
                    // cout << table.front().symbol << ": " << table.front().value << endl;
                    // cout << table.size() << endl;
                    ident.clear();
                }
                else {
                    cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    cout << "syntactic error: label for the EQU directive does not exist" << endl;
                }
                break;
            case d_IF:      // cout << token << endl;
                            break;
            default:        // nota: mexer com Link aqui
                if (token.front() == ';') {
                    // aqui o endereco nao pode contar
                    while (tokenizer >> token);
                } else {
                    it = table;
                    while (!it.empty()) {
                        // cout << it.front().symbol << ": " << it.front().value << endl;
                        relation = it.front();
                        if (token == relation.symbol) {
                            // substitoiar
                            cout << "unique" << endl;
                        }
                        it.pop();
                    }
                }
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
