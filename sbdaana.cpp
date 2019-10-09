

// ----------------------------------------------------------------------------------------------------
//    BIBLIOTECAS
// ----------------------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
using namespace std;

// ----------------------------------------------------------------------------------------------------
//    VARIAVEIS AUXILIARES
// ----------------------------------------------------------------------------------------------------

static string line;             // guarda uma linha do codigo fonte ou pre-processado
static string symbol;           // auxiliar para guardar um token
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

// criar class counter: relacionar numero da linha do arquivo preprocessado com o arquivo fonte
// class Counter {
// };

// classe para marcar secao atual no processo de analise
class Bookmark {
    public:
    int placement = s_null;
    int text_count = 0;
    int data_count = 0;
    bool error = false;

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
static Bookmark cursor;

//classe dos simbolos
class simbolo {
    public:
    queue <int> lista;
    string simbolo;
    int valor;
    bool definido;
}

vector<simbolo> tabelasimbolos; //tabela de simbolos

simbolo auxiliar;

    //pega uma linha, le palavra a palavra




// classe para criar tabela de simbolos
class Link {
    public:
    vector<Link> t_body;    // corpo da tabela
    string symbol;          // identificador ou rotulo
    string value;           // sinonimo ou simbolo

    // insere elementos na tabela
    void insert (string symbol, string value) {
        // insere elementos em uma linha
        this->symbol = symbol;
        this->value = value;

        // insere linha na tabela
        Link aux;
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
            // se palavra estiver na tabela, retorna achou
            if (word == symbol)
                return 1;
        } // se nao estiver na tabela, retorna nao achou
        return 0;
    }

    // limpa toda a tabela
    void clear () {
        symbol.clear();
        value.clear();
        t_body.clear();
    }
};
static Link table;

// ----------------------------------------------------------------------------------------------------
//    ANALISE DE CODIGO
// ----------------------------------------------------------------------------------------------------

// classe de metodos de analise de codigo
class Analyze {
    public:
    string aux;

    // inicializa, acessa, verifica ou exclui conteudo do objeto
    void insert (string token) { aux = token; }
    string content () { return aux; }
    int empty () { return aux.empty(); }
    void clear () { aux.clear(); }

    // analise: verifica se ha mais de um rotulo na mesma linha (usa auxiliar ja inicializado)
    int multiple_labels (istringstream* tokenizer, string* token) {
        if ( aux.empty() ) return 0;        // auxiliar precisa ser inicializado fora da funcao
        string* label = &aux;               // se auxiliar nao estiver vazio, label recebe seu endereco

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
    // analise: verifica validade dos rotulos inseridos no codigo fonte (usa ou cria auxiliar)
    void check_label (string* file_name, istringstream* tokenizer, string* token) {
        bool init = false;
        if ( aux.empty() ) {    // se auxiliar nao foi inicializado
            aux = *token;       // auxiliar recebe token
            init = true;        // sinaliza se auxiliar foi inicializado dentro da funcao
        }
        string label = aux;     // label recebe auxiliar

        // se label for igual a uma palavra reservada
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
        if (multiple_labels (tokenizer, token)) {
            cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
            cout << "syntactic error: more than one label on the same line" << endl;
        }

        // se auxiliar foi inicializado dentro da funcao, limpar auxiliar
        if (init) aux.clear();
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
    // analise: localiza secao atual e incrementa sua contagem
    void inside_section (istringstream* tokenizer, string* token) {
        if ( !tokenizer->eof() ) {              // se linha nao acabou
            *tokenizer >> *token;               // pega proximo token
            outline.push_back(*token);          // insere token na lina de saida

            switch ( SECTION[*token] ) {
                case s_TEXT:                    // se sucessor for TEXT
                    cursor.placement = s_TEXT;  // marcador recebe secao TEXT
                    cursor.text_count++;        // incrementa numero de secoes TEXT encontradas
                    break;
                case s_DATA:                    // se sucessor for DATA
                    cursor.placement = s_DATA;  // marcador recebe secao DATA
                    cursor.data_count++;
                    break;
                default:                        // se sucessor for invalido
                    cursor.placement = s_null;  // marcador recebe secao nenhuma
                    cursor.error = true;        // sinaliza erro
                    break;
            }
        } else {                                // se linha acabou
            cursor.placement = s_null;          // marcador recebe secao nenhuma
            cursor.error = true;                // sinaliza erro
        }
    }
    // analise: verifica a validade da secao declarada
    void check_section (string* file_name, istringstream* tokenizer, string* token) {
        inside_section(tokenizer, token);

        // se houver erro em SECTION
        if (cursor.error) {
            // se cursores ja foram definidos, erro de tentativa de sobrecarregamento de secoes
            if ( cursor.full() ) {
                cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                cout << "syntactic error: more than two sections" << endl;
                cursor.error = false;   // limpa erro para proxima avaliacao
            } // se ha cursor disponivel, tipo de secao esta ausente
            else {
                cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                cout << "syntactic error: missing section type" << endl;
                cursor.error = false;   // limpa erro para proxima avaliacao
            } // se nao houver erro, verificar se ha sobrecarregamento
        } else if ( cursor.overflowed(*token) ) {
            cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
            cout << "syntactic error: " << *token << " section already exists" << endl;
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
    Analyze label;
    int tinhalabelanterior = 0;
    int definicao, i, endaux, achou;

    // line_number++;


    while (!token.eof()){
      if(tinhalabelanterior == 0){
        tokenizer >> token;
      }
        // cout << token << endl;

        // switch ( DIRECTIVE[token] ) {
        //     case d_SECTION:
        //         tokenizer >> token;
        //         switch ( SECTION[token] ) {
        //             case s_TEXT: break; // aqui devera vir o switch para opcodes
        //             case s_DATA: break;
        //         } break;
        //     case d_SPACE:   // cout << token << endl;
        //                     break;
        //     case d_CONST:   // cout << token << endl;
        //                     break;
        //     default:
        //         break;
        // }

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
                tinhalabelanterior = 1;
                definicao = 0;
                // se token for rotulo
                if ( token.back() == ':' ) {
                    definicao = 1;       //se tiver : eh definicao
                    // cout << address << ' ' << token << endl;
                    token.pop_back();
                    static Analyze label;
                    label.check_label (file_name, &tokenizer, &token);
                    address++; // precisa ajustar para space
                }
                break;


                // nao tenho certeza, mas acho q vem aqui
                achou=0;

                for(i=0; i<tabelasimbolos.size(); i++){    // vai varrer o vector da tabela de simbolos buscando o token (nao eh endereco ou instrucao)

                    if (tabelasimbolos[i].simbolo == label.content()){        //encontrou
                        achou = 1;                                  //flag = ja tem esse simbolo na tabela

                        if(tabelasimbolos[i].definido == false){    // se nesse lugar da tabela o a definicao for false, adicionar endereco na fila
                            if(definicao==1){
                                while(!tabelasimbolos[i].lista.empty())
                                    endaux = tabelasimbolos[i].lista.front(); // auxiliar recebe a frente da fila
                                    tabelasimbolos[i].lista.pop(); //retira a frente
                                    // vai na saida e coloca nesse endereco o valor correto
                                }
                                tabelasimbolos[i].valor = //o endereço certo;
                                tabelasimbolos[i].definido = true; //agora esta definido
                            }
                            if(definicao==0){
                                tabelasimbolos[i].lista.push(//endereco)
                            }
                        }

                        if(tabelasimbolos[i].definido == true) { //se ja estava definido
                            //o endereco pro arquivo saida - se esse simbolo encontrado ja for a definicao, coloca esse endereco no arquivo de saida
                        }


                        break;
                    }

                }

                if (achou==0){      // se nao achou na tabela, vai colocar na tabela
                    auxiliar.simbolo = label.content(); //simbolo


                    if (definicao==1){          //se tiver :, eh definicao
                        auxiliar.valor = //o endereco;
                        auxiliar.definido = true;
                    }
                    if(definicao==0){           //se nao tiver :, eh uso
                        auxiliar.fila.push(//o endereco); - endereco
                        auxiliar.definido = false;
                    }

                }
                tabelasimbolos.push_back(auxiliar);
                }
        }
    }

    //pesquisar onde
}

// ----------------------------------------------------------------------------------------------------
//    PRE-PROCESSAMENTO
// ----------------------------------------------------------------------------------------------------

// funcao auxiliar de pre-processamento: escreve codigo pre-processado
void write_preprocessed_file (ofstream* pre_file) {
    // enquanto linha de saida nao acabou
    for (unsigned int i=0; i < outline.size(); i++) {

        // se houver quebra de linha, pula para proxima palavra
        if (outline[i] == newline) {
            do { i++; } while (outline[i] == newline);
            *pre_file << endl;
        }

        if ( table.search (outline[i]) )        // procura palavra na tabela de identificadores
            *pre_file << table.value << " ";    // se achou, escreve valor no arquivo de saida
        else
            *pre_file << outline[i] << " ";     // se nao achou, escreve palavra no arquivo de saida
    }
    outline.clear();    // ao final, limpar linha de saida e
    table.clear();      // limpar tabela de identificadores
}

// funcao auxiliar de pre-processamento: remove comentarios
void clear_comment (istringstream* tokenizer, string* token) {
    if (token->front() == ';') {
        outline.pop_back();             // remove comentario da linha de saida
        while (*tokenizer >> *token);   // pula para proxima linha do codigo fonte
        // nota: aqui o endereco nao pode contar
    }
}

// funcao auxiliar de pre-processamento: remove diretiva EQU da linha de saida
void clear_EQU_line (istringstream* tokenizer, string* token, bool ident_empty) {
    if (!ident_empty) {                                         // se existe identificador
        for (int i=0; i<2; i++) outline.pop_back();             // remove EQU e constante da linha de saida
        while (outline.back() == newline) outline.pop_back();   // remove quebras de linha acima, se existirem
        outline.pop_back();                                     // remove identificador
    } else {                                // se identificador estiver vazio
        outline.pop_back();                 // remove EQU da linha de saida
        while (*tokenizer >> *token);       // ignora o resto da linha
    }
}

// pre-processamento
void preprocessing (string* file_name) {
    Analyze word;                   // token a ser analisado
    istringstream tokenizer {line}; // decompositor de linha
    string token;                   // string lida na linha de entrada
    line_number++;

    while (tokenizer >> token) {    // enquanto linha nao acabou, pega um token
        outline.push_back(token);   // insere token na lina de saida

        // se token for diretiva SECTION, verificar localizacao e contagem
        switch ( DIRECTIVE[token] ) {
            case d_SECTION:
                word.inside_section (&tokenizer, &token);
            default: break;
        }

        // se houver mais de um identificador, pular para o ultimo
        if (token.back() == ':') {
            token.pop_back();                           // descarta ':'
            word.insert(token);                         // forca inicializacao de word.content()
            word.multiple_labels (&tokenizer, &token);  // checa quantidade de identificadores
            clear_comment (&tokenizer, &token);         // limpa comentario, se existir, apos identificador
            if ( tokenizer.eof() ) {                    // se houver quebra de linha apos identificador
                symbol = word.content();                // guarda identificador em symbol
                break;                                  // sai do laco
            }
        } // se nao houver identificador na linha atual, verifica a existencia de identificador na linha anterior
        else if ( !symbol.empty() ) {
            word.insert(symbol);
            symbol.clear();
        }

        // analisa o token (o token seguinte ao identificador, se existir identificador)
        switch ( DIRECTIVE[ token ] ) {
            case d_EQU:

                // se diretiva EQU aparece apos secoes declaradas, erro
                if ( cursor.got_in() ) {
                    cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    cout << "semantic error: EQU directive after section declaration" << endl;
                    cursor.error = true;    // sinaliza erro, para impressao de apenas uma mensagem de erro
                }

                // se identificador nao estiver vazio, inserir numa tabela relacionando-o com seu sinonimo
                if ( !word.empty() ) {
                    word.check_label (file_name, &tokenizer, &token);   // analisa validade do identificador
                    word.check_const (file_name, token);                // analisa validade da constante
                    clear_EQU_line (&tokenizer, &token, word.empty());  // remove diretiva EQU da linha de saida
                    if (table.search( word.content() )) {               // se encontrar identificador de mesmo nome na tabela de relacoes, erro
                        cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                        cout << "semantic error: \"" << word.content() << "\" identifier has already been declared" << endl;
                    } else                                              // se nao
                        table.insert (word.content(), token);           // insere identificador na tabela
                    word.clear();                                       // limpa identificador para proxima linha
                }

                // se identificador estiver vazio, erro na diretiva EQU
                else if ( !cursor.error ) {
                    cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    cout << "syntactic error: label for the EQU directive does not exist" << endl;
                    clear_EQU_line (&tokenizer, &token, word.empty());  // remove diretiva EQU da linha de saida
                    cursor.error = true;                                // sinaliza erro, para impressao de apenas uma mensagem de erro
                } else
                    clear_EQU_line (&tokenizer, &token, word.empty());  // se identificador estiver vazio, mas mensagem de erro ja foi imprimida, apenas apagar linha

                // se tokenizer capturar outro token diferente do guardado em symbol apos declaracao EQU, erro
                symbol = token;
                while (tokenizer >> token);
                if (!cursor.error && (token != symbol)) {
                    cout << endl << "Line " << line_number << " of [" << *file_name << "]:" << endl;
                    cout << "syntactic error: EQU directive has too many arguments" << endl;
                } else                      // se nao capturar
                    cursor.error = false;   // limpa erro para proxima avaliacao
                symbol.clear();             // limpa symbol para proxima linha
                // nota: se houver mais tokens apos declaracao e o ultimo token ainda for igual a symbol, o erro existente nao sera verificado (o que nao eh bom); solucao: substituir este ultimo bloco por arvore de parsing?

                break;

            case d_IF:
                // cout << token << endl;
                break;

            default:
                // se token for comentario, pular para proxima linha
                clear_comment (&tokenizer, &token);
                break;
        }
    }
    outline.push_back (newline);    // insere quebra de linha na linha de saida
}

// ----------------------------------------------------------------------------------------------------
//    MONTADOR
// ----------------------------------------------------------------------------------------------------

int main () {
    stringSwitch();     // inicializa palavras reservadas

    // abre arquivo contendo o codigo fonte
    string file_name = "testing.asm"; // nota: essa string depois vai ser o argumento iniciado junto ao programa na linha de comando
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
        } // nota: limpar o Bookmark
        file.close();
    }
    else cout << endl << "ERROR: File not found!";

    write_preprocessed_file (&pre_file);
    pre_file.close();

    // cout << line_number << endl;
    return 0;
}
