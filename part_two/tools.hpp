// ----------------------------------------------------------------------------------------------------
//    FERRAMENTAS
// ----------------------------------------------------------------------------------------------------

#ifndef _H_TOOLS_
#define _H_TOOLS_

// bibliotecas
#include <fstream>
#include <sstream>
#include <vector>

// constantes
#define newline "\n"

// variaveis
static string input_line;
static vector<string> output_line;
static vector<int> output_code;
static int line_number = 0;
static int program_address = 0;

// variaveis temporarias
static vector<string> previous_label;

// classe para marcar posicoes no processo de analise
class Marker {
    public:
    int begin_count = 0;
    int end_count = 0;

    void clear () {
        begin_count = 0;
        end_count = 0;
    }
}; static Marker cursor;

// classes para criacao de tabelas de simbolos
class Table_row {
    private:
        string label;           // rotulo do simbolo
        int value = -1;         // endereco do simbolo
        bool defined = false;   // indica se endereco do simbolo foi definido
        bool external = false;  // indica se o simbolo eh externo
        vector<int> list;       // lista de enderecos em que o rotulo eh chamado
    public: friend class Table;
};
class Table {
    public:
        vector<Table_row> t_body;
    
    // procura simbolo na tabela de simbolos
    int search (string label, unsigned int *i) {
        for (*i=0; *i < t_body.size(); *i++) {
            if (label == t_body[*i].label)
                return 1;
        } return 0;
    }

    // insere simbolo externo na tabela
    void insert_external (string label) {
        Table_row aux_row;
        unsigned int i = 0;
        aux_row.label = label;
        aux_row.external = true;
        t_body.push_back (aux_row);
    }

    // insere simbolo definido ou externo na tabela
    int insert_defined (string label, int value) {
        Table_row aux_row;
        unsigned int i = 0;
        aux_row.label = label;
        aux_row.value = value;
        aux_row.defined = true;

        if (search (label, &i) == 1) {      // se nao, se simbolo ja estiver na tabela
            t_body[i] = aux_row;            // insere novo valor na tabela
            return 1;                       // retorna "atualizar linhas anteriores"
        } else                              // se simbolo ainda nao estiver na tabela
            t_body.push_back (aux_row);     // insere simbolo no final da tabela
        return 0;
    }

    // insere um simbolo chamado na tabela
    int insert_called (string label, int address) {
        Table_row aux_row;
        unsigned int i = 0;
        aux_row.label = label;
        aux_row.list.push_back (address);

        if (search (label, &i) == 1) {                  // se simbolo ja estiver na tabela
            if (t_body[i].defined)                      // se simbolo for definido
                return 1;                               // retorna "escrever valor definido no arquivo de saida"
            else                                        // se simbolo for indefinido
                t_body[i].list.push_back (address);     // insere endereco atual na lista de enderecos do simbolo
        } else                              // se simbolo nao esta na tabela
            t_body.push_back (aux_row);     // insere simbolo no final da tabela
        return 0;
    }
}; static Table symbol;

// tratamento de erros
void error_handling (string *FILE_NAME, string token, int error_id) {
    switch (error_id) {
        case 1:
            cout << endl << "Linha " << line_number << " de [ " << *FILE_NAME << " ]:" << endl;
            cout << "Erro: existencia de diretiva " << token << " em programa unico" << endl;
            break;
        case 2:
            cout << endl << "Linha " << line_number << " de [ " << *FILE_NAME << " ]:" << endl;
            cout << "Erro: multiplas declaracoes de diretiva " << token << endl;
            break;
        case 3:
            cout << endl << "Linha " << line_number << " de [ " << *FILE_NAME << " ]:" << endl;
            cout << "Erro: diretiva END nao encontrada" << endl;
            break;
        case 4:
            cout << endl << "Linha " << line_number << " de [ " << *FILE_NAME << " ]:" << endl;
            cout << "Erro: declaracao de diretiva END sem declaracao previa de diretiva BEGIN" << endl;
            break; 
        default: break;
    }
}

// passagens do montador
#include "preprocessing.hpp"
#include "singlepass.hpp"

#endif
