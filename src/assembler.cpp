// ----------------------------------------------------------------------------------------------------
//    BIBLIOTECAS
// ----------------------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
using namespace std;

// ----------------------------------------------------------------------------------------------------
//    VARIAVEIS AUXILIARES
// ----------------------------------------------------------------------------------------------------

static string line;             // guarda uma linha do codigo fonte ou pre-processado 
static string token_aux;        // auxiliar para guardar um token
static int line_number = 0;     // conta a posicao da linha no codigo fonte
static int address = 0;         // conta a posicao de memoria do token
static vector<string> outline;  // linha do codigo de saida
static const string newline = "\n";     // identificador de nova linha para codigo de saida

// ----------------------------------------------------------------------------------------------------
//    DEFINICAO DE OPERACOES E DIRETIVAS RECONHECIDAS PELO MONTADOR
// ----------------------------------------------------------------------------------------------------

enum e_OPCODE {ADD=1, SUB, MULT, DIV, JMP, JMPN, JMPP, JMPZ, COPY, LOAD, STORE, INPUT, OUTPUT, STOP};
enum e_DIRECTIVE {d_SECTION=1, d_SPACE, d_CONST, d_EQU, d_IF};
enum e_SECTION {s_TEXT=1, s_DATA, s_null};
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

// // classe para relacionar numero da linha do arquivo preprocessado com o arquivo fonte
// class Counter {
//     public:
//     int asm_line;
//     int pre_line;
// };
// static vector<Counter> count;

// classe para marcar secao atual no processo de analise
class Marker {
    public:
    int placement = s_null;
    int text_count = 0;
    int data_count = 0;
    bool error = false;
    bool invalid_type = false;

    // retorna 1 caso cursor ja tenha passado por alguma secao valida
    int got_in () {
        if ((text_count > 0) || (data_count > 0))
            return 1;
        else
            return 0;
    }
    // retorna 1 se as duas secoes, TEXT e DATA ja foram definidas
    int full () {
        if ((text_count > 0) && (data_count > 0))
            return 1;
        else
            return 0;
    }
    // retorna 1 se cursor encontrar nova secao de um tipo ja declarado anteriormente
    int overflowed (string s_type) {
        switch (SECTION[ s_type ]) {
        case s_TEXT:
            if (text_count > 1)
                return 1;
        case s_DATA:
            if (data_count > 1)
                return 1;
        default:
            return 0;
        }
    }
    // limpa cursor
    void clear () {
        placement = s_null;
        text_count = 0;
        data_count = 0;
        error = false;
    }
};
static Marker cursor;

// classe para criar tabelas
class Table;            // linha da tabela
vector<Table> t_body;   // corpo da tabela
class Table {
    public:
    string symbol;      // identificador ou rotulo
    string value;       // sinonimo ou constante

    // insere elementos na tabela
    void insert (string symbol, string value) {
        // insere elementos em uma linha
        this->symbol = symbol;
        this->value = value;

        // insere linha na tabela
        Table aux;
        aux.symbol = symbol;
        aux.value = value;
        t_body.push_back (aux);
    }

    // procura uma palavra na tabela
    int search (string word) {
        // enquanto tabela nao acabou
        for (unsigned int i=0; i < t_body.size(); i++) {
            symbol = t_body[i].symbol;
            value = t_body[i].value;
            if (word == symbol) // se palavra estiver na tabela
                return 1;       // retorna achou
        }                       // se nao estiver na tabela
        return 0;               // retorna nao achou
    }

    // limpa toda a tabela
    void clear () {
        symbol.clear();
        value.clear();
        t_body.clear();
    }
};
static Table ident_table;

// ----------------------------------------------------------------------------------------------------
//    ANALISE DE CODIGO
// ----------------------------------------------------------------------------------------------------

// classe de metodos de analise de codigo
class Analyze {
    string aux;
    public:
    
    // inicializa, acessa, verifica ou exclui conteudo do objeto
    void insert (string token) { aux = token; }
    string content () { return aux; }
    int empty () { return aux.empty(); }
    void clear () { aux.clear(); }

    // analise: verifica se ha mais de um rotulo na mesma linha (usa auxiliar inicializado)
    int multiple_labels (istringstream* tokenizer, string* token) {
        if ( aux.empty() ) return 0;    // auxiliar precisa ser inicializado fora da funcao
        string* label = &aux;           // se auxiliar nao estiver vazio, label recebe seu endereco

        // se linha nao acabou, pega proximo token
        if (!tokenizer->eof() && (*tokenizer >> *token)) {
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
                return 1;                           // retorna erro
            }
        } // nao retorna erro
        return 0;
    }

    // analise: verifica validade dos rotulos inseridos no codigo fonte (usa ou cria auxiliar)
    int check_label (string* file_name, string token, int there_are_multiple_labels) {
        bool init = false;
        if ( aux.empty() ) {    // se auxiliar nao foi inicializado
            aux = token;        // auxiliar recebe token
            init = true;        // sinaliza se auxiliar foi inicializado dentro da funcao
        }
        string label = aux;     // label recebe auxiliar

        // se label for igual a uma palavra reservada, erro
        if (((OPCODE[label] >= 1) && (OPCODE[label] <= 14)) || ((DIRECTIVE[label] >= 1) && (DIRECTIVE[label] <= 5)) || ((SECTION[label] >= 1) && (SECTION[label] <= 2))) {
            std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
            std::cout << "semantic error: invalid label, \"" << label << "\" is a keyword" << endl;
            return 0;
        } else {
            // se o rotulo eh maior que 50 caracteres: erro
            if (label.size() > 50) {
                std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                std::cout << "lexicon error: label is longer than 50 characters:" << endl;
                while (outline.back().size() > 50) {
                    outline.back().pop_back();
                } label = outline.back();
                std::cout << "\t\"" << label << "\"" << endl;
                return 0;
            }
            // se o primeiro caractere for um numero: erro
            if ((label.front() >= 48) && (label.front() <= 57)) {
                std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                std::cout << "lexicon error: label \"" << label << "\" starts with a number" << endl;
                return 0;
            }
            // se o rotulo nao eh composto apenas por letras, numeros e underscore: erro
            for (unsigned int i = 0; i < label.size(); i++)
                if ((label[i] != 95) && (!((label[i] >= 48) && (label[i] <= 57)) && !((label[i] >= 65) && (label[i] <= 90)))) {
                    std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    std::cout << "lexicon error: label \"" << label << "\" is not just letters, numbers or underscore" << endl;
                    return 0;
                }
            // verifica se ha mais de um rotulo na mesma linha
            if (there_are_multiple_labels) {
                std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                std::cout << "syntactic error: more than one label on the same line" << endl;
            }
        }
        if (init) aux.clear();  // se auxiliar foi inicializado dentro da funcao, limpar auxiliar
        return 1;               // se nenhum erro foi encontrado, retorna 1
    }

    // analise: verifica validade dos rotulos e se ha mais de um rotulo na mesma linha (cria e limpa auxiliar)
    int super_check_label (string* file_name, istringstream* tokenizer, string* token) {
        insert(*token);
        int mtl = multiple_labels (tokenizer, token);
        return check_label (file_name, *token, mtl);
    }

    // analise: verifica a validade de uma constante
    int check_const (string* file_name, string token) {
        string aux;

        // se numero estiver na base hexadecimal
        if ((token.size() > 2) && (token[0] == '0') && (token[1] == 'X')) {
            token.erase (0,2);
            aux.append("0x"+token);
            for (unsigned int i=0; i < token.size(); i++) {
                // se numero nao contem apenas digitos numericos e caracteres de A a F, erro
                if (!((token[i] >= 48) && (token[i] <= 57)) && !((token[i] >= 41) && (token[i] <= 46))) {
                    std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    std::cout << "lexicon error: constant \"" << aux << "\" consisting of invalid characters" << endl;
                    return 0;
                }
            }
        } else {
            // se numero for negativo, guardar sinal
            if (token[0] == '-') {
                token.erase (0,1);
                aux.append("-");
            }
            // se numero for inteiro
            aux.append(token);
            for (unsigned int i=0; i < token.size(); i++) {
                // se numero nao contem apenas digitos numericos, erro
                if (!((token[i] >= 48) && (token[i] <= 57))) {
                    std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    std::cout << "lexicon error: constant \"" << aux << "\" consisting of invalid characters" << endl;
                    return 0;
                }
            }
        } // se nenhum erro foi encontrado, retorna 1
        return 1;
    }

    // analise: verifica validade de argumento passado a diretiva IF 
    void check_token (string* file_name, string token) {
        int valid_const;

        if ((token[0] == '-') || ((token[0] >= 48) && (token[0] <= 57))) {  // se token for numero
            valid_const = check_const (file_name, token);                   // verifica validade da constante
            if (valid_const) outline.push_back (token);                     // se for valida, insere na linha de saida
        } else if (ident_table.search (token)) {                            // se for identificador, procura na tabela
            outline.push_back (ident_table.value);                          // se achou, insere valor na linha de saida
        } else {                                                            // se nao achou, erro
            std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
            std::cout << "semantic error: label \"" << token << "\" has not been declared" << endl;
            outline.pop_back();                                             // remove IF da linha de saida
        }
    }
    // analise: localiza secao atual e incrementa sua contagem
    void inside_section (istringstream* tokenizer, string* token) {
        // se linha nao acabou, pega proximo token
        if (!tokenizer->eof() && (*tokenizer >> *token)) {
            outline.push_back(*token);          // insere token na lina de saida

            switch ( SECTION[*token] ) {
                case s_TEXT:                    // se sucessor for TEXT
                    cursor.placement = s_TEXT;  // marcador recebe secao tipo TEXT
                    cursor.text_count++;        // incrementa numero de secoes TEXT encontradas
                    break;
                case s_DATA:                    // se sucessor for DATA
                    cursor.placement = s_DATA;  // marcador recebe secao tipo DATA
                    cursor.data_count++;
                    break;
                default:                        // se sucessor for invalido
                    cursor.placement = s_null;  // marcador recebe tipo vazio
                    cursor.error = true;        // sinaliza erro
                    cursor.invalid_type = true;
                    break;
            }
        } else {                                // se linha acabou
            cursor.placement = s_null;          // marcador recebe tipo vazio
            cursor.error = true;                // sinaliza erro
        }
    }
    // analise: verifica a validade da secao declarada
    void check_section (string* file_name, istringstream* tokenizer, string* token) {    
        inside_section(tokenizer, token);

        // se houver erro em SECTION
        if (cursor.error) {
            std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
            // se secoes ja foram definidas, erro de tentativa de sobrecarregamento de secoes
            if ( cursor.full() ) {
                std::cout << "syntactic error: more than two sections" << endl;
                cursor.error = false;   // limpa erro para proxima avaliacao
            } // se achou tipo desconhecido, erro de tipo invalido
            else if (cursor.invalid_type) {
                std::cout << "semantic error: " << *token << " is an invalid section type" << endl;
                cursor.invalid_type = false;
                cursor.error = false;
            // se tipo nao foi definido, erro de tipo de secao ausente
            } else {
                std::cout << "syntactic error: missing section type" << endl;
                cursor.error = false;   // limpa erro para proxima avaliacao
            } // se nao houver erro, verificar se tipo ja foi declarado anteriormente
        } else if ( cursor.overflowed(*token) ) {
            std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
            std::cout << "syntactic error: " << *token << " section already exists" << endl;
        }
    }
};

// ----------------------------------------------------------------------------------------------------
//    SINTESE DE CODIGO
// ----------------------------------------------------------------------------------------------------

// passagem unica
void onepass (string* file_name) {
    Analyze label;
    istringstream tokenizer {line};
    string token;
    line_number++;

    while (tokenizer >> token) {
        // std::cout << token << endl;

        // switch ( DIRECTIVE[token] ) {
        //     case d_SECTION:
        //         tokenizer >> token;
        //         switch ( SECTION[token] ) {
        //             case s_TEXT: break; // aqui devera vir o switch para opcodes
        //             case s_DATA: break;
        //         } break;
        //     case d_SPACE:   // std::cout << token << endl;
        //                     break;
        //     case d_CONST:   // std::cout << token << endl;
        //                     break;
        //     default:
        //         break;
        // }

        switch ( OPCODE[token] ) {
            case ADD:       // std::cout << token << endl;
                            break;
            case SUB:       // std::cout << token << endl;
                            break;
            case MULT:      // std::cout << token << endl;
                            break;
            case DIV:       // std::cout << token << endl;
                            break;
            case JMP:       // std::cout << token << endl;
                            break;
            case JMPN:      // std::cout << token << endl;
                            break;
            case JMPP:      // std::cout << token << endl;
                            break;
            case JMPZ:      // std::cout << token << endl;
                            break;
            case COPY:      // std::cout << token << endl;
                            break;
            case LOAD:      // std::cout << token << endl;
                            break;
            case STORE:     // std::cout << token << endl;
                            break;
            case INPUT:     // std::cout << token << endl;
                            break;
            case OUTPUT:    // std::cout << token << endl;
                            break;
            case STOP:      // std::cout << token << endl;
                            break;
            default:
                // se token for rotulo
                if ( token.back() == ':' ) {
                    // std::cout << address << ' ' << token << endl;
                    token.pop_back();
                    label.super_check_label (file_name, &tokenizer, &token);
                    address++; // precisa ajustar para space
                }
                break;
        }
    }
}

// ----------------------------------------------------------------------------------------------------
//    PRE-PROCESSAMENTO
// ----------------------------------------------------------------------------------------------------

// funcao auxiliar de pre-processamento: move SECTION DATA para baixo de SECTION TEXT
int change_places (ofstream* pre_file, unsigned int i, bool* text_sign, bool* data_sign) {

    // recebe palavra SECTION e se linha de saida nao acabou, pega proxima palavra
    if ((i+1) != outline.size()) i++;
    while (i < outline.size()) {
        if (!(*text_sign) || !(*data_sign)) {       // se falta escrever algum tipo de secao
            switch ( DIRECTIVE[ outline[i-1] ] ) {  // se palavra anterior for SECTION
                case d_SECTION:
                    switch ( SECTION[ outline[i] ] ) {

                        case s_DATA:                        // se palavra atual for DATA
                            outline[i] = "SECTION DATA";    // palavra atual recebe SECTION DATA
                            *data_sign = true;              // sinaliza ja escreveu SECTION DATA
                            break;                          // vai para a condicao final

                        case s_TEXT:                        // se palavra atual for TEXT
                            if (data_sign)                  // se moveu SECTION DATA para o final
                                outline.pop_back();         // apaga palavra SECTION do final
                            i--;                            // retorna para palavra SECTION
                            *pre_file << outline[i] << " "; // escreve SECTION no arquivo de saida
                            *text_sign = true;              // sinaliza ja escreveu SECTION TEXT
                            return i;                       // retorna indice de continuacao de escrita

                        default:                                    // para qualquer outra palavra atual
                            if (!(*data_sign)) {                    // se ainda nao escreveu SECTION DATA
                                *pre_file << outline[i-1] << " ";   // escreve SECTION no arquivo de saida
                                return i;                           // retorna indice de continuacao
                            } else                                  // se escreveu SECTION DATA
                                break;                              // vai para condicao final
                    }
                default: break;
            }
        }
        if (*data_sign)                         // se estiver em DATA, mas ainda nao achou TEXT
            outline.push_back ( outline[i] );   // move palavra atual para o final da linha de saida
        i++;
    }
    return i;
}

// pre-processamento: escreve codigo pre-processado
void write_preprocessed_file (ofstream* pre_file) {
    cursor.clear();             // limpa o cursor para a passagem unica
    bool text_sign = false;     // sinaliza se escreveu SECTION TEXT
    bool data_sign = false;     // sinaliza se escreveu SECTION DATA

    // enquanto linha de saida nao acabou
    for (unsigned int i=0; i < outline.size(); i++) {

        // se houver quebra de linha, pula para proxima palavra
        if (outline[i] == newline) {
            if ((i+1) != outline.size())
                do { i++; } while (((i+1) != outline.size()) && (outline[i] == newline));
            *pre_file << endl;
        }

        // se palavra for rotulo, escreve no arquivo de saida e pula para proxima palavra
        while (outline[i].back() == ':') {
            *pre_file << outline[i] << " ";
            if ((i+1) != outline.size())
                do { i++; } while (((i+1) != outline.size()) && (outline[i] == newline));
            else break;
        }

        switch ( DIRECTIVE[ outline[i] ] ) {

            case d_SECTION:         // se palavra for diretiva SECTION
                if (!text_sign)     // se ainda nao escreveu SECTION TEXT, verificar tipo da secao
                    i = change_places(pre_file, i, &text_sign, &data_sign);
                else                // se escreveu SECTION TEXT, escreve SECTION no arquivo de saida
                    *pre_file << outline[i] << " ";
                break;

            case d_IF:                              // se palavra for diretiva IF
                if ((i+1) != outline.size()) i++;   // pula para proxima palavra
                if (outline[i] != "1") {            // se palavra for diferente de '1'
                    // ignora quebra de linha e pula para proxima palavra
                    for (int j=0; j < 2; j++)
                        if ((i+1) != outline.size()) i++;
                    // se linha de saida ainda nao acabou, ignora a instrucao seguinte
                    while (((i+1) != outline.size()) && (outline[i] != newline)) i++;
                } else {                                // se palavra for igual a '1'
                    if ((i+1) != outline.size()) i++;   // ignora quebra de linha
                }
                break;

            default:                                        // se palavra nao for diretiva IF
                if ( ident_table.search (outline[i]) )      // procura palavra na tabela de identificadores
                    *pre_file << ident_table.value << " ";  // se achou, escreve valor no arquivo de saida
                else                                        // se nao achou
                    *pre_file << outline[i] << " ";         // escreve palavra no arquivo de saida
                break;
        }
    }
    outline.clear();        // ao final, limpar linha de saida e
    ident_table.clear();    // limpar tabela de identificadores
}

// funcao auxiliar de pre-processamento: remove comentarios
int clear_comment (istringstream* tokenizer, string* token, bool inside_outline) {
    if (token->front() == ';') {
        if (inside_outline)             // se comentario foi inserido na linha de saida
            outline.pop_back();         // remove comentario da linha de saida
        while (*tokenizer >> *token);   // pula para proxima linha do codigo fonte
        return 1;
    }
    return 0;
}

// funcao auxiliar de pre-processamento: remove diretiva EQU da linha de saida
void clear_EQU_line (istringstream* tokenizer, string* token, bool ident_empty) {
    if (!ident_empty) {                                         // se existe identificador
        outline.pop_back();                                     // remove EQU da linha de saida
        while (outline.back() == newline) outline.pop_back();   // remove quebras de linha acima, se existirem
        outline.pop_back();                                     // remove identificador
    } else {                                // se identificador estiver vazio
        outline.pop_back();                 // remove EQU da linha de saida
        while (*tokenizer >> *token);       // ignora o resto da linha
    }
}

// pre-processamento: funcao principal
void preprocessing (string* file_name) {
    Analyze word;                   // token a ser analisado
    istringstream tokenizer {line}; // decompositor de linha
    string token;                   // string lida na linha de entrada
    int mtl = 0;                    // sinal de retorno de multiple_labels
    line_number++;

    while (tokenizer >> token) {    // enquanto linha nao acabou, pega um token
        outline.push_back(token);   // insere token na lina de saida

        // se token for diretiva SECTION, verificar localizacao e contagem
        switch ( DIRECTIVE[token] ) {
            case d_SECTION:
                word.inside_section (&tokenizer, &token);
            default: break;
        }

        // se token for identificador
        if (token.back() == ':') {
            token.pop_back();                                   // descarta ':'
            word.insert(token);                                 // forca inicializacao de word.content(): passa token para word.aux
            mtl = word.multiple_labels (&tokenizer, &token);    // verifica se ha mais de um identificador na mesma linha
            clear_comment (&tokenizer, &token, true);           // limpa comentario, se existir, apos identificador
            if ( tokenizer.eof() ) {                            // se houver quebra de linha apos identificador
                token_aux = word.content();                     // guarda identificador em token_aux
            }
        }
        // se nao houver identificador na linha atual, verifica a existencia de identificador na linha anterior
        else if ( !token_aux.empty() ) {
            word.insert(token_aux);
            token_aux.clear();
        }

        // analisa o token (o token seguinte ao identificador, se existir identificador)
        switch ( DIRECTIVE[ token ] ) {
            case d_EQU:

                // se diretiva EQU aparece apos secoes declaradas, erro
                if ( cursor.got_in() ) {
                    std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    std::cout << "semantic error: EQU directive after section declaration" << endl;
                }

                // se identificador nao estiver vazio e nao for repetido, inserir numa tabela relacionando-o com seu sinonimo
                if ( !word.empty() ) {
                    if (word.check_label (file_name, token, mtl)) {     // analisa validade do identificador
                        // se linha nao acabou, pega a constante se token nao for comentario
                        if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false)) {
                            word.check_const (file_name, token);            // analisa validade da constante
                            if (ident_table.search (word.content()) ) {     // se identificador ja estava na tabela, erro
                                std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                                std::cout << "semantic error: label \"" << word.content() << "\" has already been declared" << endl;
                            } else                                          // se identificador nao esta na tabela
                                ident_table.insert (word.content(), token); // insere identificador na tabela
                        } else {                                            // se linha acabou, erro
                            std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                            std::cout << "syntactic error: missing constant in EQU statement" << endl;
                        }
                    } else while (tokenizer >> token);                  // se identificador for invalido, ignorar linha
                    clear_EQU_line (&tokenizer, &token, word.empty());  // remove diretiva EQU da linha de saida
                    word.clear();                                       // limpa identificador para proxima linha
                }

                // se identificador estiver vazio, erro na diretiva EQU
                else {
                    std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    std::cout << "syntactic error: label for the EQU directive does not exist" << endl;
                    clear_EQU_line (&tokenizer, &token, word.empty());  // remove diretiva EQU da linha de saida
                }

                // se linha nao acabou, captura proximo token, se token for comentario, pula linha, se nao erro
                if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false)) {
                    std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    std::cout << "syntactic error: EQU directive has too many arguments" << endl;
                    while (tokenizer >> token);     // ignora o restante da linha
                }
                break;

            case d_IF:

                // se linha nao acabou, pega proximo token, se token for comentario, pular linha
                if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false))
                    word.check_token (file_name, token);    // se nao for comentario, analisa token
                else {                                      // se nao ha identificador ou constante, erro
                    std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    std::cout << "syntactic error: missing argument in IF statement" << endl;
                    outline.pop_back();
                }

                // se linha nao acabou, pega proximo token, se token for comentario, pula linha, se nao erro
                if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false)) {
                    std::cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    std::cout << "syntactic error: IF directive has too many arguments" << endl;
                    while (tokenizer >> token);     // ignora o restante da linha
                }
                break;

            default:
                // se token for comentario, pular para proxima linha
                clear_comment (&tokenizer, &token, true);
                break;
        }
    }
    outline.push_back (newline);    // insere quebra de linha na linha de saida
}

// ----------------------------------------------------------------------------------------------------
//    MONTADOR
// ----------------------------------------------------------------------------------------------------

// inicio
int main () {
    stringSwitch();     // inicializa palavras reservadas

    // abre arquivo contendo o codigo fonte
    string file_name = "testing.asm"; // nota: essa string depois vai ser o argumento iniciado junto ao programa na linha de comando
    ifstream file (file_name);

    // cria um arquivo contendo o codigo pre-processado
    string aux_name = file_name;
    for (int i=0; i<3; i++) aux_name.pop_back();
    string pre_name, obj_name;
    pre_name.append(aux_name+"pre");
    obj_name.append(aux_name+"obj");

    // le codigo assembly e faz o pre-processamento
    if ( file.is_open() ) {
        while ( !file.eof() ) {                     // enquanto arquivo nao acabou
            getline (file, line);                   // le linha do codigo fonte
            for (auto & c: line) c = toupper(c);    // retira sensibilidade ao caso
            preprocessing (&file_name);             // realiza o pre-processamento
        } file.close();

        // escreve o codigo pre-processado
        ofstream out_file (pre_name);
        write_preprocessed_file (&out_file);
        out_file.close();

        file.open (pre_name);
        line_number = 0;            // nota: atualizar depois para linha do codigo fonte
        while ( !file.eof() ) {     // enquanto arquivo nao acabou
            getline (file, line);   // le linha do codigo pre-processado
            // onepass (&pre_name);    // realiza primeira passagem
        } file.close();
    }
    else std::cout << endl << "ERROR: File not found!" << endl;

    return 0;
}
