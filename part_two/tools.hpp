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
static int line_number;

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

#endif
