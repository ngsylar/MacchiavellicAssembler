// ----------------------------------------------------------------------------------------------------
//    FERRAMENTAS
// ----------------------------------------------------------------------------------------------------

#ifndef _H_TOOLS_
#define _H_TOOLS_

// bibliotecas
#include <fstream>      // arquivos
#include <sstream>      // leitura de arquivos e strings
#include <cstring>      // strings
#include <vector>       // vetores

// constantes
#define newline "\n"

// variaveis
static string input_line;               // linha lida de um arquivo de entrada
static vector<string> output_line;      // linha de saida para arquivo pre-processado
static vector<int> output_code;         // linha de saida para arquivo objeto
static int line_number = 0;             // contador de linhas
static int program_address = 0;         // contador de enderecos

// classe para marcar posicoes no processo de analise
class Marker {
    public:
    vector<string> module_name;     // nome do modulo
    int module_index = 1;           // indica o modulo atual
    int begin_count = 0;            // marca o numero de diretivas BEGIN achado em um arquivo
    int end_count = 0;              // marca se achou uma diretiva END em um arquivo

    void clear () {
        begin_count = 0;
        end_count = 0;
    }
}; static Marker cursor;

// classes para criacao de tabelas de simbolos
class Table_row {
    public:
        string label;           // rotulo do simbolo
        int value = -1;         // endereco do simbolo
        bool defined = false;   // indica se endereco do simbolo foi definido
        bool shared = false;    // indica se o simbolo eh publico
        bool external = false;  // indica se o simbolo eh externo
        vector<int> list;       // lista de enderecos em que o rotulo eh chamado

    // reinicia simbolo
    void clear () {
        label.clear();
        value = -1;
        defined = false;
        shared = false;
        external = false;
        list.clear();
    }
    friend class Table;
};
class Table {
    public:
        int current_i;              // indice de uma linha na tabela
        Table_row current;          // linha auxiliar com elemento salvo na posicao "indice" da tabela
        vector<Table_row> t_body;   // corpo completo da tabela

    // retorna a lista de enderecos de um simbolo na tabela
    void current_list (vector<int> *list) {
        *list = t_body[current_i].list;
    }

    // procura simbolo na tabela de simbolos
    int search (string label) {
        for (unsigned int i=0; i < t_body.size(); i++) {    // para cada simbolo na tabela
            if (label == t_body[i].label) {                 // se rotulo na tabela for igual a rotulo recebido
                current = t_body[i];                        // linha auxiliar recebe simbolo encontrado
                current_i = i;                              // guarda o indice da linha
                return 1;                                   // retorna "achou"
            }           // se ao final da busca, nenhum simbolo com rotulo igual foi encontrado
        } return 0;     // retorna "nao achou"
    }

    // insere valor na lista de enderecos do simbolo atual
    void insert_address (int address) {
        t_body[current_i].list.push_back(address);
    }

    // insere um simbolo publico na tabela
    void insert_public (string label) {
        if (search (label) == 1) {                  // se simbolo ja estiver na tabela
            if (!t_body[current_i].shared)
                t_body[current_i].shared = true;    // declara simbolo como publico
        }
        else {                              // se simbolo nao esta na tabela
            current.clear();                // cria nova linha
            current.label = label;          // declara rotulo do simbolo
            current.shared = true;          // marca como publico
            t_body.push_back (current);     // insere simbolo no final da tabela
        }
    }
    // insere simbolo externo na tabela
    void insert_external (string label) {
        if (search (label) == 1) {                  // se simbolo ja estiver na tabela
            if (!t_body[current_i].external)
                t_body[current_i].external = true;  // declara simbolo como externo
        }
        else {                              // se simbolo nao esta na tabela
            current.clear();                // cria nova linha
            current.label = label;          // declara rotulo do simbolo
            current.external = true;        // marca como externo
            t_body.push_back (current);     // insere simbolo no final da tabela;
        }
    }
    // insere simbolo definido ou externo na tabela
    int insert_defined (string label, int value) {        
        if (search (label) == 1) {              // se nao, se simbolo ja estiver na tabela
            t_body[current_i].value = value;    // insere novo valor na tabela
            t_body[current_i].defined = true;   // marca simbolo como definido
            current = t_body[current_i];        // salva simbolo atualizado na linha auxiliar
            return 1;                           // retorna "atualizar linhas anteriores"
        }
        else {                              // se simbolo ainda nao estiver na tabela
            current.clear();                // cria nova linha
            current.label = label;          // declara rotulo do simbolo
            current.value = value;          // insere valor definido
            current.defined = true;         // marca como definido
            t_body.push_back (current);     // insere simbolo no final da tabela
        } return 0;
    }

    // insere um simbolo nao definido na tabela
    void insert_undefined (string label, int address) {
        current.clear();                    // cria nova linha
        current.label = label;              // declara rotulo do simbolo
        current.list.push_back (address);   // insere endereco atual na lista de enderecos do simbolo
        t_body.push_back (current);         // insere linha no final da tabela
    }

    // limpa a tabela de simbolos
    void clear () {
        current_i = 0;
        current.clear();
        t_body.clear();
    }
}; static Table symbol;

// transforma string com representacao decimal ou hexadecimal em inteiro
int hdstoi (string token) {
    int number;                                     // valor de retorno

    if ((token[0] == '0') && (token[1] == 'x')) {   // se for hexadecimal
        char stoc[ token.size() ];                  // cria array de char com tamanho da string
        strcpy(stoc, token.c_str());                // copia string para o array de char
        number = strtol(stoc, NULL, 16);            // numero recebe array convertido em inteiro
    } else                                          // se for decimal
        number = stoi(token);                       // numero recebe string convertida em inteiro

    return number;                                  // retorna numero
}

// analise de codigo
#include "analysis.hpp"

// sintese de codigo
#include "preprocessing.hpp"
#include "singlepass.hpp"

#endif
