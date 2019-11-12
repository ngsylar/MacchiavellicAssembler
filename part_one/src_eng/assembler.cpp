// ----------------------------------------------------------------------------------------------------
//    BIBLIOTECAS
// ----------------------------------------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <map>
using namespace std;

// ----------------------------------------------------------------------------------------------------
//    VARIAVEIS AUXILIARES
// ----------------------------------------------------------------------------------------------------

static string line;                     // guarda uma linha do codigo fonte ou pre-processado 
static string token_aux;                // auxiliar para guardar um token
static int line_number = 0;             // conta a posicao da linha no codigo fonte
static int address = 0;                 // conta a posicao de memoria do token
static vector<string> outline;          // linha de saida: contem todo o codigo pre-processado
static vector<int> objline;             // linha objeto: contem todo o codigo objeto
static const string newline = "\n";     // identificador de nova linha para codigo de saida
static const string markline = "$";    // identificador de nova linha para codigo de saida

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
//    FUNCOES AUXILIARES
// ----------------------------------------------------------------------------------------------------

// transforma string com representacao decimal ou hexadecimal em inteiro
int hrstoi (string token) {
    int number;                                     // valor de retorno

    if ((token[0] == '0') && (token[1] == 'X')) {   // se for hexadecimal
        char stoc[ token.size() ];                  // cria array de char com tamanho da string
        strcpy(stoc, token.c_str());                // copia string para o array de char
        number = strtol(stoc, NULL, 16);            // numero recebe array convertido em inteiro
    } else                                          // se for decimal
        number = stoi(token);                       // numero recebe string convertida em inteiro

    return number;                                  // retorna numero
}

// transforma int em string
string itos (int number) {
    stringstream token;
    token << number;
    string word = token.str();
    return word;
}

// ----------------------------------------------------------------------------------------------------
//    CLASSES AUXILIARES
// ----------------------------------------------------------------------------------------------------

// classe para relacionar numero da linha do arquivo preprocessado com o arquivo fonte
class Counter;
vector<Counter> t_count;
class Counter {
    public:
    int asm_line = 0;
    int pre_line = 0;

    // conta uma linha do arquivo fonte e salva temporariamente seu numero em outline
    void lines () {
        line_number++;
        string aux = markline;
        aux.append( itos(line_number) );
        outline.push_back(aux);
    }

    // insere linha no codigo pre-processado, guardando a linha equivalente do codigo fonte
    void insert_line (ofstream* pre_file) {
        Counter aux;
        pre_line++;

        aux.asm_line = this->asm_line;
        aux.pre_line = this->pre_line;
        t_count.push_back(aux);
        *pre_file << endl;
    }

    // se houver quebra de linha ou numero da linha, retorna 1
    int line_break (unsigned int i) {
        if ((outline[i] == newline) || (outline[i].front() == markline.front())) {
            if ((outline[i].front() == markline.front())) {
                string aux = outline[i];
                aux.erase(0,1);
                asm_line = stoi(aux)-1;
            } return 1;
        } else return 0;
    }

    // ve numero da linha do arquivo lido e retorna numero correspondente no arquivo fonte
    string check_line (string file_name) {
        string reading, aux;
        
        for (int i=0; i < 3; i++) {
            reading.insert(0, &file_name.back());
            file_name.pop_back();
        }
        if (reading == "asm") {
            aux.append (itos(line_number) + " of [ " + file_name+"asm" + " ]");
            return aux;
        } else {
            int i = 0;
            do {
                asm_line = t_count[i].asm_line;
                pre_line = t_count[i].pre_line;
                i++;
            } while (line_number != pre_line);
            aux.append (itos(asm_line) + " of [ " + file_name+"asm" + " ]");
            return aux;
        }
    }
    // ve numero da linha do arquivo lido e retorna numero correspondente no arquivo fonte
    int post_lines (int reading_line) {
        int i = 0;
        do {
            asm_line = t_count[i].asm_line;
            pre_line = t_count[i].pre_line;
            i++;
        } while (reading_line != pre_line);
        return asm_line;
    }
};
static Counter count;

// classe para marcar secao atual no processo de analise
class Marker {
    public:
    int placement = s_null;
    int text_begin = -1;
    int data_begin = -1;
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
        text_begin = -1;
        data_begin = -1;
        text_count = 0;
        data_count = 0;
        error = false;
        invalid_type = false;
    }
};
static Marker cursor;

// classe para criar tabelas
class Table;            // linha da tabela
vector<Table> t_body;   // corpo da tabela
class Table {
    public:
        string symbol;          // identificador ou rotulo
        string value;           // sinonimo ou constante
        int stance = -1;        // endereco do rotulo
        bool def = false;       // indica se endereco do rotulo foi definido
        vector<int> list;       // lista de enderecos em que o simbolo aparece
        vector<int> line_list;  // lista de linhas em que o simbolo parece
        int index;              // indice do rotulo na tabela

    // insere elementos na tabela de identificadores
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

    // insere elementos na tabela de simbolos
    void insert (string symbol, int stance, bool def) {
        // insere elemento em uma linha
        this->symbol = symbol;
        this->stance = stance;
        this->def = def;

        // insere linha na tabela
        Table aux;
        aux.symbol = symbol;
        aux.def = def;

        // se for definicao, limpa lista e adiciona endereco verdadeiro
        if (def) {
            aux.stance = stance;
            aux.list.clear();
            aux.line_list.clear();
        }
        // se nao for definicao, salva endereco de aparicao na lista
        else {
            aux.stance = -1;
            aux.list.push_back (stance);
            aux.line_list.push_back (line_number);
        }
        // guarda na tabela
        t_body.push_back (aux);
    }

    // insere endereco na na lista de um simbolo
    void insert_list (int position) {
        t_body[index].list.push_back (position);
        t_body[index].line_list.push_back (line_number);
    }

    // procura uma palavra na tabela
    int search (string word) {
        // enquanto tabela nao acabou
        for (unsigned int i=0; i < t_body.size(); i++) {
            symbol = t_body[i].symbol;
            value = t_body[i].value;
            stance = t_body[i].stance;
            def = t_body[i].def;
            index = i;
            if (word == symbol) // se palavra estiver na tabela
                return 1;       // retorna achou
        }                       // se nao estiver na tabela
        return 0;               // retorna nao achou
    }

    // define endereco do simbolo como verdadeiro e substitui todas as linhas da lista por esse endereco
    void validate (string symbol, int stance) {
        unsigned int i=0, j=0;

        if (search(symbol)) {                           // procura simbolo na tabela
            t_body[index].stance = stance;              // guarda numero do endereco real
            t_body[index].def = true;                   // define endereco como verdadeiro
            while (!(t_body[index].list.empty())) {     // enquanto lista do simbolo nao acabou
                i = t_body[index].list.size() - 1;      // i recebe ultima posicao da lista
                j = t_body[index].list[i];              // j recebe endereco salvo na ultima posicao da lista
                if (j < objline.size())                 // se j for menor que tamanho da saida
                    objline[j] = objline[j] + stance;   // insere numero do endereco real no ultimo endereco guardado na lista
                t_body[index].list.pop_back();          // remove ultima posicao da lista e volta o laco
            }
        }
    }
    // procura todas os simbolos indefinidos na tabela
    void search_undefined (string* file_name) {
        // enquanto tabela nao acabou
        for (unsigned int i=0; i < t_body.size(); i++) {
            symbol = t_body[i].symbol;
            def = t_body[i].def;
            // se achou simbolo indefinido, mostra todas as linhas que simbolo aparece
            if (def == false) {
                std::cout << endl << "Lines { ";
                for (unsigned int j=0; j < t_body[i].line_list.size(); j++)
                    std::cout << count.post_lines (t_body[i].line_list[j]) << " ";
                std::cout << "} of [" << *file_name << "]:" << endl;
                std::cout << "semantic error: label \"" << symbol << "\" is undefined" << endl;
            }
        }
    }
    // limpa toda a tabela
    void clear () {
        symbol.clear();
        value.clear();
        stance = -1;
        def = false;
        list.clear();
        line_list.clear();
        t_body.clear();
    }
};
static Table ident_table;
static Table symbol_table;

// ----------------------------------------------------------------------------------------------------
//    ANALISE DE CODIGO
// ----------------------------------------------------------------------------------------------------

// classe com listas de enderecos para analisar instrucoes
class Informant {
    // todas as listas guardam as linhas [0] e os enderecos [1] onde as operacoes ocorrem
    vector<int> div_list[2];    // o valor na memoria nao pode ser zero 0
    vector<int> jmp_list[2];    // o operando nao pode ser maior ou igual a cursor.data_begin
    vector<int> set_list[2];    // para COPY, STORE e INPUT, o operando nao pode apontar para valor constante
    vector<int> const_list;     // apenas guarda enderecos onde um valor constante aparece

    public:
    // insere elementos numa lista de acordo com a operacao
    void insert (int operation) {
        if (operation == DIV) {
            div_list[0].push_back (line_number);
            div_list[1].push_back (address);
        } else if ((JMP <= operation) && (operation <= JMPZ)) {
            jmp_list[0].push_back (line_number);
            jmp_list[1].push_back (address);
        } else if ((operation == COPY) || (operation == STORE) || (operation == INPUT)) {
            set_list[0].push_back (line_number);
            set_list[1].push_back (address);
        }
    }
    // insere endereco na lista de constantes
    void insert_const () {
        const_list.push_back (address);
    }

    // verificar se lista esta vazia
    int empty (vector<int> list[]) {
        return list[0].empty();
    } // acessar memoria apontada por endereco
    int iterate (vector<int> list[]) {
        int i, j;
        i = list[1].front();
        j = objline[i];
        return j;
    } // apagar primeiro elemento da lista
    void pop_front (vector<int> list[]) {
        list[0].erase (list[0].begin());
        list[1].erase (list[1].begin());
    }

    // procura um endereco na lista de constantes
    int search_const (int position) {
        for (unsigned int i=0; i < const_list.size(); i++) {
            if (position == const_list[i])
                return 1;
        }
        return 0;
    }

    // mostrar erros contidos nas listas
    void post (string* file_name) {
        int i;

        // lista de DIV
        while ( !empty(div_list) ) {
            i = iterate(div_list);
            if (objline[i] == 0) {
                std::cout << endl << "Lines { ";
                while ( !empty(div_list) ) {
                    i = iterate(div_list);
                    if (objline[i] == 0)
                        std::cout << count.post_lines (div_list[0].front()) << " ";
                    pop_front (div_list);
                }
                std::cout << "} of [" << *file_name << "]:" << endl;
                std::cout << "semantic error: division by zero" << endl;
                break;
            }
            pop_front (div_list);
        }
        // lista de JMPs
        if (cursor.data_begin >= 0)
            while ( !empty(jmp_list) ) {
                i = jmp_list[1].front();
                if (objline[i] >= cursor.data_begin) {
                    std::cout << endl << "Lines { ";
                    while ( !empty(jmp_list) ) {
                        i = jmp_list[1].front();
                        if (objline[i] >= cursor.data_begin)
                            std::cout << count.post_lines (jmp_list[0].front()) << " ";
                        pop_front (jmp_list);
                    }
                    std::cout << "} of [" << *file_name << "]:" << endl;
                    std::cout << "semantic error: jump to SECTION DATA" << endl;
                    break;
                }
                pop_front (jmp_list);
            }
        // lista de COPY, STORE e INPUT
        while ( !empty(set_list) ) {
            i = set_list[1].front();
            if (search_const (objline[i])) {
                std::cout << endl << "Lines { ";
                while ( !empty(set_list) ) {
                    i = set_list[1].front();
                    if (search_const (objline[i]))
                        std::cout << count.post_lines (set_list[0].front()) << " ";
                    pop_front (set_list);
                }
                std::cout << "} of [" << *file_name << "]:" << endl;
                std::cout << "semantic error: constant value modification" << endl;
                break;
            }
            pop_front (set_list);
        }
        const_list.clear();
    }
};
static Informant report;

// classe de metodos de analise de tokens
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

    // analise: verifica validade dos rotulos inseridos no codigo fonte (usa ou incializa e limpa auxiliar)
    int check_label (string* file_name, string token, int there_are_multiple_labels) {
        bool init = false;
        if ( aux.empty() ) {    // se auxiliar nao foi inicializado
            aux = token;        // auxiliar recebe token
            init = true;        // sinaliza se auxiliar foi inicializado dentro da funcao
        }
        string label = aux;     // label recebe auxiliar

        // se label for igual a uma palavra reservada, erro
        if (((OPCODE[label] >= 1) && (OPCODE[label] <= 14)) || ((DIRECTIVE[label] >= 1) && (DIRECTIVE[label] <= 5)) || ((SECTION[label] >= 1) && (SECTION[label] <= 2))) {
            std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
            std::cout << "semantic error: invalid label, \"" << label << "\" is a keyword" << endl;
            return 0;
        } else {
            // se o rotulo eh maior que 50 caracteres: erro
            if (label.size() > 50) {
                std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                std::cout << "lexicon error: label is longer than 50 characters:" << endl;
                std::cout << "\t\"" << label << "\"" << endl;
                return 0;
            }
            // se o primeiro caractere for um numero: erro
            if ((label.front() >= 48) && (label.front() <= 57)) {
                std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                std::cout << "lexicon error: label \"" << label << "\" starts with a number" << endl;
                return 0;
            }
            // se o rotulo nao eh composto apenas por letras, numeros e underscore: erro
            for (unsigned int i = 0; i < label.size(); i++)
                if ((label[i] != 95) && (!((label[i] >= 48) && (label[i] <= 57)) && !((label[i] >= 65) && (label[i] <= 90)))) {
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                    std::cout << "lexicon error: label \"" << label << "\" is not just letters, numbers or underscore" << endl;
                    return 0;
                }
            // verifica se ha mais de um rotulo na mesma linha
            if (there_are_multiple_labels) {
                std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                std::cout << "syntactic error: more than one label on the same line" << endl;
            }
        }
        if (init) aux.clear();  // se auxiliar foi inicializado dentro da funcao, limpar auxiliar
        return 1;               // se nenhum erro foi encontrado, retorna 1
    }

    // analise: verifica validade dos rotulos e se ha mais de um rotulo na mesma linha (inicializa auxiliar)
    int super_check_label (string* file_name, istringstream* tokenizer, string* token) {
        insert(*token);
        int mtl = multiple_labels (tokenizer, token);
        return check_label (file_name, *token, mtl);
    }

    // analise: verifica a validade de um inteiro
    int check_nat (string* file_name, string token) {
        for (unsigned int i=0; i < token.size(); i++) {
            // se numero nao contem apenas digitos numericos, erro
            if (!((token[i] >= 48) && (token[i] <= 57))) {
                std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                std::cout << "lexicon error: constant \"" << aux << "\" consisting of invalid characters" << endl;
                return 0;
            }
        } // se nenhum erro foi encontrado, retorna 1
        return 1;
    }

    // analise: verifica a validade de um numero
    int check_number (string* file_name, string token) {
        string aux;

        // se numero estiver na base hexadecimal
        if ((token.size() > 2) && (token[0] == '0') && (token[1] == 'X')) {
            token.erase (0,2);
            aux.append("0x"+token);
            for (unsigned int i=0; i < token.size(); i++)
                // se numero nao contem apenas digitos numericos e caracteres de A a F, erro
                if (!((token[i] >= 48) && (token[i] <= 57)) && !((token[i] >= 41) && (token[i] <= 46))) {
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                    std::cout << "lexicon error: constant \"" << aux << "\" consisting of invalid characters" << endl;
                    return 0;
                }
        } else {
            // se numero for negativo, guardar sinal
            if (token[0] == '-') {
                token.erase (0,1);
                aux.append("-");
            }
            // se numero for inteiro, analisa numero
            aux.append(token);
            return check_nat (file_name, token);
        }
        // se nenhum erro foi encontrado, retorna 1
        return 1;
    }

    // analise: verifica validade de argumento passado a diretiva IF 
    void check_token (string* file_name, string token) {
        int valid_number;

        if ((token[0] == '-') || ((token[0] >= 48) && (token[0] <= 57))) {  // se token for numero
            valid_number = check_number (file_name, token);                 // verifica validade do numero
            if (valid_number) outline.push_back (token);                    // se for valido, insere na linha de saida
        } else if (ident_table.search (token)) {                            // se for identificador, procura na tabela
            outline.push_back (ident_table.value);                          // se achou, insere valor na linha de saida
        } else {                                                            // se nao achou, erro
            std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
            std::cout << "semantic error: label \"" << token << "\" has not been declared" << endl;
            outline.pop_back();                                             // remove IF da linha de saida
        }
    }
    // analise: verifica validade de operando passado a uma instrucao 
    void check_operand (string* file_name, string token, int position, bool label_only) {
        int valid_const;
        objline.push_back(0);

        // se nao for para analisar apenas rotulo e token for numero
        if ((!label_only) && ((token[0] == '-') || ((token[0] >= 48) && (token[0] <= 57)))) {
            valid_const = check_number (file_name, token);  // verifica validade do numero
            if (valid_const)                                // se for valido
                objline.back() += hrstoi(token);            // insere na saida
        }
        // se for rotulo, procura na tabela de simbolos
        else if (symbol_table.search (token)) {
            if (symbol_table.def) {                         // se achou rotulo definido
                objline.back() += symbol_table.stance;      // inserir endereco na saida
            } else {                                        // se achou rotulo indefinido
                symbol_table.insert_list (position);        // insere endereco na lista do simbolo
            }
        } else {                                            // se nao achou rotulo na tabela
            symbol_table.insert (token, position, false);   // insere rotulo indefinido na tabela
        }
    }
    // analise: separa argumentos dentro de uma expressao
    void check_expression (string* file_name, string expression, int position) {
        istringstream tokenizer {expression};
        vector<string> tokens;

        // separa expressao em sub-operandos
        while (getline(tokenizer, expression, '+')) {
            tokens.push_back (expression);
        }
        // se houver apenas um sub-operando, analisa como rotulo ou numero
        if (tokens.size() == 1) {
            check_operand (file_name, tokens.front(), position, false);
        }
        // se houver mais de um operando, analisa um rotulo e depois um numero natural
        else if (tokens.size() > 1) {
            check_operand (file_name, tokens[0], position, true);   // verifica rotulo
            if (check_nat (file_name, tokens[1])) {                 // se natural for valido
                objline.back() += stoi(tokens[1]);                  // soma endereco do rotulo e numero na saida
            }                                                       // se natural for invalido, nao realiza soma
            // se houver mais de dois operandos, erro
            if (tokens.size() > 2) {
                std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                std::cout << "syntactic error: operand receiving too many arguments" << endl;
            }
        }
    }
    // analise: verifica existencia de multiplos argumentos na mesma instrucao
    void multiple_arguments (string* file_name, istringstream* tokenizer, string* token, string instruction) {
        if (!tokenizer->eof() && (*tokenizer >> *token)) {
            std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
            std::cout << "syntactic error: " << instruction << " receiving too many arguments" << endl;
            while (*tokenizer >> *token);   // ignora o restante da linha
        }
    }
    // analise: localiza secao atual e incrementa sua contagem
    void inside_section (istringstream* tokenizer, string* token) {
        // se linha nao acabou, pega proximo token
        if (!tokenizer->eof() && (*tokenizer >> *token)) {
            outline.push_back(*token);  // insere token na lina de saida

            switch ( SECTION[*token] ) {
                case s_TEXT:                            // se sucessor for TEXT
                    if (cursor.text_count == 0) {       // se ainda nao existe SECTION TEXT
                        cursor.placement = s_TEXT;      // marcador recebe secao tipo TEXT
                        cursor.text_begin = address;    // marcador recebe linha de inicio de SECTION TEXT
                    } cursor.text_count++;              // incrementa numero de secoes TEXT encontradas
                    break;

                case s_DATA:                            // se sucessor for TEXT
                    if (cursor.data_count == 0) {       // se ainda nao existe SECTION TEXT
                        cursor.placement = s_DATA;      // marcador recebe secao tipo TEXT
                        cursor.data_begin = address;    // marcador recebe linha de inicio de SECTION TEXT
                    } cursor.data_count++;              // incrementa numero de secoes TEXT encontradas
                    break;

                default:                        // se sucessor for invalido
                    cursor.error = true;        // sinaliza erro
                    cursor.invalid_type = true;
                    break;
            }
        } else {                    // se linha acabou
            cursor.error = true;    // sinaliza erro
        }
    }
    // analise: verifica a validade da secao declarada
    void check_section (string* file_name, istringstream* tokenizer, string* token) {    
        inside_section(tokenizer, token);

        // se houver erro em SECTION
        if (cursor.error) {
            std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
            // se secoes ja foram definidas, erro de tentativa de sobrecarregamento de secoes
            if ( cursor.full() ) {
                std::cout << "syntactic error: section overflow" << endl;
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
            std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
            std::cout << "syntactic error: SECTION " << *token << " already exists" << endl;
        } // se houver outro token apos declaracao, erro
        multiple_arguments (file_name, tokenizer, token, "SECTION directive");
    }

    // analise: verifica se operacao esta fora da secao de texto
    void outside_text (string* file_name, string token) {
        if (cursor.placement != s_TEXT) {
            std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
            std::cout << "semantic error: " << token << " operation outside SECTION TEXT" << endl;
        }
    }
    // analise: verifica se diretiva esta fora da secao de dados
    void outside_data (string* file_name, string token) {
        if (cursor.placement != s_DATA) {
            std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
            std::cout << "semantic error: " << token << " directive outside SECTION DATA" << endl;
        }
    }
};

// ----------------------------------------------------------------------------------------------------
//    SINTESE DE CODIGO
// ----------------------------------------------------------------------------------------------------

// passagem unica: escreve codigo objeto
void write_object_file (ofstream* obj_file, string* file_name) {
    report.post (file_name);                    // mostra erros em instrucoes
    symbol_table.search_undefined (file_name);  // mostra rotulos que nao foram definidos

    // enquanto saida nao acabou, escreve no arquivo final
    for (unsigned int i=0; i < objline.size(); i++) {
        *obj_file << objline[i] << " ";
    }
    cursor.clear();         // limpa o marcador
    objline.clear();        // limpa a saida
    symbol_table.clear();   // limpa a tabela de simbolos
}

// funcao auxiliar de passagem unica: processa uma operacao
void process_opcode (string* file_name, istringstream* tokenizer, string* token, Analyze* word, int rule) {
    string operation = *token;

    // se linha nao acabou, pega operando
    if (!tokenizer->eof() && (*tokenizer >> *token)) {
        word->check_expression (file_name, *token, address);                            // verifica validade da expressao e insere na saida
        word->multiple_arguments (file_name, tokenizer, token, operation+" operation"); // verifica se ha sobrecarga de operandos
        if (((DIV <= rule) && (rule <= JMPZ)) || (rule == STORE) || (rule == INPUT))
            report.insert (rule);
        address++;
    }
    // se linha acabou, erro
    else {
        std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
        std::cout << "syntactic error: missing operand in " << operation << " instruction" << endl;
    }
}

// funcao auxiliar de passagem unica: processa operacao COPY
void process_copy (string* file_name, istringstream* tokenizer, string* token, Analyze* word) {
    int overflow = 0;

    // se linha nao acabou, pega operandos
    if (!tokenizer->eof() && (*tokenizer >> *token)) {
        string expression = *token;
        istringstream parser {expression};
        vector<string> operands;

        // separa operandos
        while (getline(parser, expression, ',')) {
            operands.push_back (expression);
        }
        // se houver apenas um operando, analisa, insere na saida e mostra erro
        if (operands.size() == 1) {
            word->check_expression (file_name, operands[0], address);
            address++;
            std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
            std::cout << "syntactic error: missing second operand in COPY operation" << endl;
        }
        // se houver mais de um operando, analisa e insere ambos na saida
        else if (operands.size() > 1) {
            word->check_expression (file_name, operands[0], address);
            address++;
            word->check_expression (file_name, operands[1], address);
            report.insert (COPY);
            address++;

            // se houver sobrecarga de operandos, erro
            if (operands.size() > 2) {
                std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                std::cout << "syntactic error: COPY operation receiving too many arguments" << endl;
                overflow = 1;
            }
        } // se nao houve erro de sobrecarga de operandos por virgula, verificar sobrecarga por espacos
        if (!overflow) {
            word->multiple_arguments (file_name, tokenizer, token, "COPY operation");
        }
    }
    else { // se nao ha operandos, erro
        std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
        std::cout << "syntactic error: missing operands in COPY instruction" << endl;
    }

}

// passagem unica: funcao principal
void onepass (string* file_name) {
    Analyze word;
    istringstream tokenizer {line};
    string token;
    line_number++;

    // enquanto linha nao acabou
    bool invalid_token = true;
    while (tokenizer >> token) {

        // se token for rotulo
        if ( token.back() == ':' ) {
            token.pop_back();                   // ignora ':'
            if (symbol_table.search (token)) {  // procura rotulo na tabela de simbolos
                if (symbol_table.def) {         // se achou e simbolo ja foi definido, erro
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                    std::cout << "semantic error: label \"" << token << "\" has already been declared" << endl;
                } else {                                            // se achou, mas ainda nao foi definido
                    symbol_table.validate (token, address);         // define simbolo na tabela de simbolos
                    if (!tokenizer.eof() && (tokenizer >> token));  // se linha nao acabou, pega proximo token
                    else {                                          // se linha acabou, erro
                        std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                        std::cout << "syntactic error: label \"" << token << "\" does not point to any memory addresses" << endl;
                    }
                }
            } else { // se nao achou rotulo na tabela de simbolos
                word.super_check_label (file_name, &tokenizer, &token); // analisa validade do rotulo
                symbol_table.insert (word.content(), address, true);    // insere rotulo definido na tabela de simbolos
            }
        }
        // se token for diretiva
        switch ( DIRECTIVE[token] ) {
            
            case d_SECTION:
                word.check_section (file_name, &tokenizer, &token); // verifica validade da secao
                invalid_token = false;
                break;

            case d_SPACE:
                word.outside_data (file_name, token);       // verifica se SPACE esta dentro de SECTION DATA
                // se linha nao acabou e proxima palavra eh numero natural, analisa
                if (!tokenizer.eof() && (tokenizer >> token) && word.check_nat (file_name, token)) {
                    for (int i=0; i < stoi(token); i++) {   // para o numero de espacos inseridos
                        objline.push_back(0);               // guarda zero
                    } address = address + stoi(token);      // incrementa endereco
                } else {                    // caso contrario
                    objline.push_back(0);   // guarda apenas um espaco
                    address++;              // incrementa endereco
                } // verifica a existencia de operandos posteriores
                word.multiple_arguments (file_name, &tokenizer, &token, "SPACE directive");
                invalid_token = false;
                break;

            case d_CONST:
                invalid_token = false;
                word.outside_data (file_name, token);   // verifica se CONST esta dentro de SECTION DATA
                // se linha nao acabou e proxima palavra eh numero
                if (!tokenizer.eof() && (tokenizer >> token)) {
                    if (word.check_number (file_name, token)) { // analisa numero
                        objline.push_back (hrstoi(token));      // se valido, insere constante na saida
                        report.insert_const();                  // guarda linha e endereco da constante
                        address++;                              // incrementa endereco
                    } // verifica a existencia de operandos posteriores
                    word.multiple_arguments (file_name, &tokenizer, &token, "CONST directive");
                } else {                    // caso contrario, erro
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                    std::cout << "syntactic error: missing constant in CONST statement" << endl;
                    objline.push_back(0);   // insere zero no lugar da constante
                    address++;              // incrementa endereco
                }
                invalid_token = false;
                break;

            default: break;
        }

        // se token for operacao
        switch ( OPCODE[token] ) {

            case ADD:
                word.outside_text (file_name, token);   // verifica se ADD esta dentro de SECTION TEXT
                objline.push_back(1);                   // insere na saida
                address++;                              // incrementa endereco
                process_opcode (file_name, &tokenizer, &token, &word, ADD);
                break;

            case SUB:
                word.outside_text (file_name, token);
                objline.push_back(2);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, SUB);
                break;

            case MULT:
                word.outside_text (file_name, token);
                objline.push_back(3);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, MULT);
                break;

            case DIV:
                word.outside_text (file_name, token);
                objline.push_back(4);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, DIV);
                break;

            case JMP:
                word.outside_text (file_name, token);
                objline.push_back(5);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, JMP);
                break;

            case JMPN:
                word.outside_text (file_name, token);
                objline.push_back(6);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, JMPN);
                break;

            case JMPP:
                word.outside_text (file_name, token);
                objline.push_back(7);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, JMPP);
                break;

            case JMPZ:
                word.outside_text (file_name, token);
                objline.push_back(8);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, JMPZ);
                break;

            case COPY:
                word.outside_text (file_name, token);
                objline.push_back(9);
                address++;
                process_copy (file_name, &tokenizer, &token, &word);
                break;

            case LOAD:
                word.outside_text (file_name, token);
                objline.push_back(10);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, LOAD);
                break;

            case STORE:
                word.outside_text (file_name, token);
                objline.push_back(11);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, STORE);
                break;

            case INPUT:
                word.outside_text (file_name, token);
                objline.push_back(12);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, INPUT);
                break;

            case OUTPUT:
                word.outside_text (file_name, token);
                objline.push_back(13);
                address++;
                process_opcode (file_name, &tokenizer, &token, &word, OUTPUT);
                break;

            case STOP:
                word.outside_text (file_name, token);
                objline.push_back(14);
                address++;
                if (!tokenizer.eof() && (tokenizer >> token)) {
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                    std::cout << "syntactic error: STOP operation receiving unexpected arguments" << endl;
                    while (tokenizer >> token);
                }
                break;

            default:
                // para qualquer operacao nao reconhecida, erro
                if (invalid_token) {
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                    std::cout << "semantic error: " << token << " is an invalid instruction" << endl;
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
void write_preprocessed_file (ofstream* pre_file, string* file_name) {
    bool text_sign = false;     // sinaliza se escreveu SECTION TEXT
    bool data_sign = false;     // sinaliza se escreveu SECTION DATA

    // se SECTION TEXT nao existe, erro
    if (cursor.text_count == 0) {
        std::cout << endl << "In [" << *file_name << "]:" << endl;
        std::cout << "syntactic error: code has missing SECTION TEXT" << endl;
    }

    // unsigned int km=0; while (km < outline.size()) {std::cout << outline[km] << " "; km++;}

    // pular quebras de linha iniciais
    while ((count.line_break(0)))
        outline.erase( outline.begin() );

    // enquanto linha de saida nao acabou
    for (unsigned int i=0; i < outline.size(); i++) {

        // se houver quebra de linha, pula para proxima palavra
        if (count.line_break(i)) {
            // ignorar quebras de linha
            if ((i+1) != outline.size()) {
                do { i++; } while (((i+1) != outline.size()) && (count.line_break(i)));
                if (count.line_break(i) && ((i+1) == outline.size())) {
                    count.insert_line (pre_file);                    
                    break;
                } // se proximo token for final do arquivo, sair do laco
            } else {
                count.insert_line (pre_file);
                break;
            } // inserir uma unica quebra de linha
            count.insert_line (pre_file);
        }

        // se palavra for rotulo, escreve no arquivo de saida e pula para proxima palavra
        while (outline[i].back() == ':') {
            *pre_file << outline[i] << " ";
            if ((i+1) != outline.size()) {
                do { i++; } while (((i+1) != outline.size()) && (count.line_break(i)));
                if (count.line_break(i) && ((i+1) == outline.size())) {
                    count.insert_line (pre_file);                    
                    break;
                }
            } else {
                count.insert_line (pre_file);
                break;
            }
        }
        switch ( DIRECTIVE[ outline[i] ] ) {

            case d_SECTION: // se palavra for diretiva SECTION, se ainda nao escreveu SECTION TEXT, verificar tipo da secao
                if (!text_sign && (cursor.text_count > 0)) {
                    i = change_places(pre_file, i, &text_sign, &data_sign);
                } else      // se escreveu SECTION TEXT, escreve SECTION no arquivo de saida
                    *pre_file << outline[i] << " ";
                break;

            case d_IF:                              // se palavra for diretiva IF
                if ((i+1) != outline.size()) i++;   // pula para proxima palavra
                if (outline[i] != "1") {            // se palavra for diferente de '1'
                    // ignora quebra de linha e pula para proxima palavra
                    if ((i+1) != outline.size()) i++;
                    while (((i+1) != outline.size()) && (outline[i+1] != newline)) i++;
                    // se linha de saida ainda nao acabou, ignora a instrucao seguinte
                    while (((i+1) != outline.size()) && (count.line_break(i+1))) i++;
                } else {    // se palavra for igual a '1' ignora quebra de linha
                    while (((i+1) != outline.size()) && (count.line_break(i+1))) i++;
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

    // km=0; while (km < t_count.size()) {std::cout << t_count[km].pre_line << " = " << t_count[km].asm_line << endl; km++;}

    cursor.clear();         // limpa o cursor para a passagem unica
    outline.clear();        // ao final, limpar linha de saida e
    ident_table.clear();    // limpar tabela de identificadores
    line_number = 0;        // limpa contagem de linhas para primeira passagem
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
    if (!ident_empty) {             // se existe identificador
        outline.pop_back();         // remove EQU da linha de saida
        while (count.line_break( outline.size()-1 ))
            outline.pop_back();     // remove quebras de linha acima, se existirem
        outline.pop_back();         // remove identificador
    } else {                            // se identificador estiver vazio
        outline.pop_back();             // remove EQU da linha de saida
        while (*tokenizer >> *token);   // ignora o resto da linha
    }
}

// pre-processamento: funcao principal
void preprocessing (string* file_name) {
    Analyze word;                   // token a ser analisado
    istringstream tokenizer {line}; // decompositor de linha
    string token;                   // string lida na linha de entrada

    count.lines();                  // conta uma linha
    int mtl = 0;                    // sinal de retorno de multiple_labels

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
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                    std::cout << "semantic error: EQU directive after section declaration" << endl;
                }

                // se identificador nao estiver vazio e nao for repetido, inserir numa tabela relacionando-o com seu sinonimo
                if ( !word.empty() ) {
                    if (word.check_label (file_name, token, mtl)) {     // analisa validade do identificador
                        // se linha nao acabou, pega um numero se token nao for comentario
                        if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false)) {
                            word.check_number (file_name, token);           // analisa validade do numero
                            if (ident_table.search (word.content()) ) {     // se identificador ja estava na tabela, erro
                                std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                                std::cout << "semantic error: label \"" << word.content() << "\" has already been declared" << endl;
                            } else                                          // se identificador nao esta na tabela
                                ident_table.insert (word.content(), token); // insere identificador na tabela
                        } else {                                            // se linha acabou, erro
                            std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                            std::cout << "syntactic error: missing constant in EQU statement" << endl;
                        }
                    } else while (tokenizer >> token);                  // se identificador for invalido, ignorar linha
                    clear_EQU_line (&tokenizer, &token, word.empty());  // remove diretiva EQU da linha de saida
                    word.clear();                                       // limpa identificador para proxima linha
                }

                // se identificador estiver vazio, erro na diretiva EQU
                else {
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                    std::cout << "syntactic error: label for EQU directive does not exist" << endl;
                    clear_EQU_line (&tokenizer, &token, word.empty());  // remove diretiva EQU da linha de saida
                }

                // se linha nao acabou, captura proximo token, se token for comentario, pula linha, se nao erro
                if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false)) {
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                    std::cout << "syntactic error: EQU directive receiving too many arguments" << endl;
                    while (tokenizer >> token);     // ignora o restante da linha
                }
                break;

            case d_IF:

                // se linha nao acabou, pega proximo token, se token for comentario, pular linha
                if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false))
                    word.check_token (file_name, token);    // se nao for comentario, analisa token
                else {                                      // se nao ha identificador ou constante, erro
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
                    std::cout << "syntactic error: missing argument in IF statement" << endl;
                    outline.pop_back();
                }

                // se linha nao acabou, pega proximo token, se token for comentario, pula linha, se nao erro
                if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false)) {
                    std::cout << endl << "Line " << count.check_line (*file_name) << ":" << endl;
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
int main (int argc, char *argv[]) {
    stringSwitch();     // inicializa palavras reservadas

    // abre arquivo contendo o codigo fonte
    string file_name;
    if (argc > 1) {
        file_name = argv[1];
    } else {
        std::cout << "Enter a file name: " ;
        std::cin >> file_name;
    }
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
        write_preprocessed_file (&out_file, &file_name);
        out_file.close();

        file.open (pre_name);
        while ( !file.eof() ) {     // enquanto arquivo nao acabou
            getline (file, line);   // le linha do codigo pre-processado
            onepass (&pre_name);    // realiza primeira passagem
        } file.close();

        // escreve o codigo objeto
        out_file.open (obj_name);
        write_object_file (&out_file, &file_name);
        out_file.close();
    }
    else std::cout << endl << "ERROR: File not found!" << endl;

    return 0;
}
