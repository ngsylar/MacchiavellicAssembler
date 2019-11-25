// ----------------------------------------------------------------------------------------------------
//    ANALISE DE CODIGO
// ----------------------------------------------------------------------------------------------------

#ifndef _H_ANALYSIS_
#define _H_ANALYSIS_

// bibliotecas
#include <cstring>

// mensagens para tratamento de erros
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
// classe para analisar tokens
class Analyze {
    public:
        string token;
        int number;

    // analisa um numero
    int check_number (string token, bool is_integer) {
        // se numero estiver na base hexadecimal
        if ((token.size() > 2) && (token[0] == '0') && (token[1] == 'X')) {
            token.erase (0,2);
            this->token.append("0x"+token);
            for (unsigned int i=0; i < token.size(); i++)
                if (!((token[i] >= 48) && (token[i] <= 57)) && !((token[i] >= 41) && (token[i] <= 46)))
                    return 0;   // se numero nao contem apenas digitos numericos e caracteres de A a F, retorna 0
        } else {
            // se numero for negativo, guardar sinal
            if (is_integer && (token[0] == '-')) {
                token.erase (0,1);
                this->token.append("-");
            } // se numero estiver na base decimal
            this->token.append(token);
            for (unsigned int i=0; i < token.size(); i++)
                if (!((token[i] >= 48) && (token[i] <= 57)))
                    return 0;   // se numero nao contem apenas digitos numericos, retorna 0
        }
        // se nenhum erro foi encontrado, salva numero analisado e retorna 1
        number = hdstoi(token);
        return 1;
    }

    // verifica se operando eh um numero ou um rotulo
    int check_operand (string token, int address, bool label_only) {
        // se token for numero
        if ((!label_only) && ((token[0] == '-') || ((token[0] >= 48) && (token[0] <= 57)))) {
            if (check_number (token, true))
                return number;
        }
        // se token for rotulo
        else if (symbol.search (token)) {
            if (symbol.current.defined)         // se achou rotulo definido
                return symbol.current.value;    // inserir endereco na saida
            else                                // se achou rotulo indefinido
                symbol.insert (address);        // insere endereco na lista do simbolo
        } else {                                        // se nao achou rotulo na tabela
            symbol.insert_undefined (token, address);   // insere rotulo indefinido na tabela
        } return 0;
    }

    // separa argumentos dentro de uma expressao
    int check_expression (string expression, int address) {
        istringstream tokenizer {expression};
        vector<string> tokens;
        int value = 0;

        // separa expressao em sub-operandos
        unsigned int i = 0;
        while (getline(tokenizer, expression, '+')) {
            tokens.push_back (expression);
            if (++i == 2) break;
        }
        // se houver apenas um sub-operando, analisa como rotulo ou numero
        if (tokens.size() == 1) {
            value += check_operand (tokens.front(), address, false);
        }
        // se houver mais de um operando, analisa um rotulo e depois um numero natural
        else if (tokens.size() > 1) {
            value += check_operand (tokens[0], address, true);  // analisa rotulo
            if (check_number (tokens[1], false))                // analisa numero
                value += number;
        }
        return value;
    }
};

#endif
