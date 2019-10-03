// ----------------------------------------------------------------------------------------------------
//    BIBLIOTECAS
// ----------------------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <list>
#include <map>
using namespace std;

// ----------------------------------------------------------------------------------------------------
//    VARIAVEIS AUXILIARES
// ----------------------------------------------------------------------------------------------------

static string line;             // guarda uma linha do codigo fonte ou pre-processado 
static string symbol;           // auxiliar para guardar um token
static int line_number = 0;     // conta a posicao da linha no codigo fonte
static int address = 0;         // conta a posicao de memoria do token
static list<string> outline;    // linha do codigo de saida

// ----------------------------------------------------------------------------------------------------
//    DEFINICAO DE OPERACOES E DIRETIVAS RECONHECIDAS PELO MONTADOR
// ----------------------------------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------------------------------
//    CLASSES AUXILIARES
// ----------------------------------------------------------------------------------------------------

// criar class counter: relacionar numero da linha do arquivo preprocessado com o arquivo fonte
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

// ----------------------------------------------------------------------------------------------------
//    ANALISE DE CODIGO
// ----------------------------------------------------------------------------------------------------

// classe de metodos de analise de codigo
class Analyze {
    public:
    string content;
    
    // inicializa, verifica ou exclui conteudo do objeto
    void insert (string token) { content = token; }
    int empty ()    { return content.empty(); }
    void clear ()   { content.clear(); }

    // analise: verifica se ha mais de um rotulo na mesma linha (usa auxiliar content)
    int check_duplicate (istringstream* tokenizer, string* token) {
        if ( content.empty() ) return 0;            // content precisa ser inicializado fora da funcao
        string* label = &content;                   // se content nao estiver vazio, label recebe seu endereco

        if ( !tokenizer->eof() ) {                  // se linha nao acabou
            *tokenizer >> *token;                   // pega o proximo token
            outline.push_back(*token);              // insere token na lina de saida
            if (token->back() == ':') {             // se token for rotulo entao
                do {                                // laco
                    token->pop_back();              // descarta ':'
                    *label = *token;                // rotulo recebe token
                    if ( !tokenizer->eof() ) {      // se linha nao acabou
                        *tokenizer >> *token;       // pega proximo token
                        outline.push_back(*token);  // insere token na lina de saida
                    } else break;                   // se linha terminou, sai do laco
                } while (token->back() == ':');     // repetir laco enquanto token for rotulo
                return 1;                           // retorna 1 se houver mais de um rotulo
            } else return 0;                        // se token nao for rotulo, retorna 0
        } else return 0;                            // se linha acabou, retorna 0
    }

    // analise: verifica validade dos rotulos inseridos no codigo fonte (usa ou cria auxiliar content)
    void check_label (string* file_name, istringstream* tokenizer, string* token) {
        bool init = false;
        if ( content.empty() ) {    // se content nao foi inicializado
            content = *token;       // content recebe conteudo do token
            init = true;            // sinaliza se content foi inicializado dentro da funcao
        }
        string label = content;     // label recebe content

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

        // se content foi inicializado dentro da funcao, limpar content
        if (init) content.clear();
    }

    // analise: verifica a validade de uma constante
    void check_const (string* file_name, string token) {
        string aux;

        // se numero for negativo, guardar sinal
        if (token.front() == '-') {
            token.erase (0,1);
            aux.append("-");
        }

        // se numero estiver na base hexadecimal
        if ((token.size() > 2) && (token[0] == '0') && (token[1] == 'X')) {
            token.erase (0,2);
            aux.append("0x"+token);
            for (unsigned int i=0; i < token.size(); i++) {
                // se numero nao contem apenas digitos numericos e caracteres de A a F, erro
                if (!((token[i] >= 48) && (token[i] <= 57)) && !((token[i] >= 41) && (token[i] <= 46))) {
                    cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    cout << "syntactic error: constant \"" << aux << "\" consisting of invalid characters" << endl;
                }
            }
        } else {
            // se numero nao for hexadecimal, analisar como decimal
            aux.append(token);
            for (unsigned int i=0; i < token.size(); i++) {
                // se numeo nao contem apenas digitos numericos, erro
                if (!((token[i] >= 48) && (token[i] <= 57))) {
                    cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    cout << "syntactic error: constant \"" << aux << "\" consisting of invalid characters" << endl;
                }
            }
        }
    }
};

// ----------------------------------------------------------------------------------------------------
//    SINTESE DE CODIGO
// ----------------------------------------------------------------------------------------------------

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
                    label.check_label (file_name, &tokenizer, &token);
                    address++; // precisa ajustar para space
                }
                break;
        }
    }
}

// ----------------------------------------------------------------------------------------------------
//    PRE-PROCESSAMENTO
// ----------------------------------------------------------------------------------------------------

// funcao auxiliar de pre-processamento: escreve codigo pre-processado
void write_preprocessed_file (ofstream* pre_file) {
    queue<Link> it;

    while (!outline.empty()) {
        if (outline.front() == ".newline.") {
            // pre_file << outline.front() << endl;
            *pre_file << endl;
        } else {
            it = table;
            try {
                while ( !it.empty() ) {
                    relation = it.front();
                    it.pop();
                    if (outline.front() == relation.symbol) {
                        throw 1;
                        break;
                    } else if ( it.empty() ) {
                        throw 0;
                    }
                }
            } catch (int linked) {
                if (linked) {
                    *pre_file << relation.value << " ";
                } else if (!linked) {
                    *pre_file << outline.front() << " ";
                }
            }
        }
        outline.pop_front();
    }
}

// funcao auxiliar de pre-processamento: remove diretiva EQU da linha de saida
void clear_EQU_line () {
    for (int i=0; i<2; i++) outline.pop_back();
    if (outline.back() == ".newline.") outline.pop_back();
    outline.pop_back();
}

// pre-processamento
void preprocessing (string* file_name) {
    Analyze ident;                  // identificador a ser analisado
    istringstream tokenizer {line}; // decompositor de linha
    string token;                   // string lida na linha de entrada
    line_number++;

    while (tokenizer >> token) {
        outline.push_back(token);   // insere token na lina de saida

        // se houver rotulos repetidos, pular para o ultimo
        if (token.back() == ':') {
            token.pop_back();
            ident.insert(token);
            ident.check_duplicate (&tokenizer, &token);
            // se houver quebra de linha apos rotulo, sai do laco (nao limpa a string token)
            if ( tokenizer.eof() || (token.front() == ';'))
                symbol = ident.content;
        } // se nao houver rotulo na linha atual, verifica a existencia de rotulo na linha anterior
        else if (!symbol.empty()) {
            ident.insert(symbol);
            symbol.clear();
        }

        // analisa o token (o token seguinte ao rotulo, se existir rotulo)
        switch ( DIRECTIVE[ token ] ) {
            case d_EQU:
                // se identificador nao estiver vazio, inserir numa tabela relacionando-o com seu sinonimo
                if (!ident.empty()) {
                    ident.check_label (file_name, &tokenizer, &token);  // analisa validade do rotulo
                    ident.check_const (file_name, token);               // analisa validade da constante
                    clear_EQU_line();                                   // remove diretiva EQU da linha de saida
                    // cout << outline.back() << token << endl;
                    relation.insert (ident.content, token);             // relaciona simbolo e valor
                    // cout << relation.symbol << " " << relation.value << endl;
                    table.push (relation);                              // insere relacao numa tabela
                    ident.clear();                                      // limpa rotulo para proximas linhas
                }
                // se estiver vazio, erro na diretiva EQU
                else {
                    cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    cout << "syntactic error: label for the EQU directive does not exist" << endl;
                    outline.pop_back();             // remove diretiva EQU da linha de saida
                    while (tokenizer >> token);     // ignora o resto da linha
                }
                break;
            case d_IF:      // cout << token << endl;
                            break;
            default:        // nota: mexer com Link aqui
                // se token for comentario, pular para proxima linha
                if (token.front() == ';') {
                    // nota: aqui o endereco nao pode contar
                    while (tokenizer >> token);
                }
                break;
        }
    }
    outline.push_back(".newline.");
}

// ----------------------------------------------------------------------------------------------------
//    MONTADOR
// ----------------------------------------------------------------------------------------------------

int main () {
    stringSwitch();     // inicializa palavras reservadas

    // abre arquivo contendo o codigo fonte
    string file_name = "testing.asm"; // essa string depois vai ser o argumento iniciado junto ao programa na linha de comando
    ifstream file (file_name);

    // cria um arquivo contendo o codigo pre-processado
    string out_name = file_name;
    for (int i=0; i<3; i++) out_name.pop_back();
    out_name.append("pre");
    ofstream pre_file (out_name);

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

    write_preprocessed_file (&pre_file);
    pre_file.close();

    // cout << line_number << endl;
    return 0;
}
