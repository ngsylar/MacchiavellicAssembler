// ----------------------------------------------------------------------------------------------------
//    LIGADOR
// ----------------------------------------------------------------------------------------------------

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;

static string SOURCE_A_NAME, SOURCE_B_NAME;
static string input_line;
static int fator_correcao_A=0, fator_correcao_B;
static string info_reloc_A, info_reloc_B;
static vector<string> uso_A, uso_B;
static string cod_obj;
static vector<int> output_line;
static string mod_name;

class Tabela {
    public:
    string simbolo;
    int valor;
};
static vector<Tabela> tg_def;

void insere_valores_finais_A () {
    istringstream tokenizer {uso_A.front()};
    string token;

    tokenizer >> token;
    for (unsigned int i=0; i < tg_def.size(); i++) {
        if (token == tg_def[i].simbolo) {
            while (tokenizer >> token) {
                token.pop_back();
                // cout << output_line[stoi(token)] << " " << fator_correcao_B << endl;
                output_line[stoi(token)] += tg_def[i].valor;
            } break;
        }
    } uso_A.erase(uso_A.begin());
}

void insere_valores_finais_B () {
    istringstream tokenizer {uso_B.front()};
    string token;

    tokenizer >> token;
    for (unsigned int i=0; i < tg_def.size(); i++) {
        if (token == tg_def[i].simbolo) {
            while (tokenizer >> token) {
                token.pop_back();
                output_line[stoi(token)+fator_correcao_B] += tg_def[i].valor;
            } break;
        }
    } uso_B.erase(uso_B.begin());
    if (!uso_B.empty()) {
        insere_valores_finais_B ();
    }
}

void pega_tg_def_A () {
    istringstream tokenizer {input_line};
    string token;
    Tabela simbaux;

    tokenizer >> token;
    simbaux.simbolo = token;
    tokenizer >> token;
    simbaux.valor = stoi(token) + fator_correcao_A;
    tg_def.push_back(simbaux);
}

void pega_tg_def_B () {
    istringstream tokenizer {input_line};
    string token;
    Tabela simbaux;

    tokenizer >> token;
    simbaux.simbolo = token;
    tokenizer >> token;
    simbaux.valor = stoi(token) + fator_correcao_B;
    tg_def.push_back(simbaux);
}

void pega_fator_correcao () {
    istringstream tokenizer {input_line};
    string token;

    tokenizer >> token;
    tokenizer >> token;
    fator_correcao_B = stoi(token);
}

void pega_info_reloc_A () {
    istringstream tokenizer {input_line};
    string token;

    tokenizer >> token;
    tokenizer >> token;
    info_reloc_A = token;
}

void pega_info_reloc_B () {
    istringstream tokenizer {input_line};
    string token;

    tokenizer >> token;
    tokenizer >> token;
    info_reloc_B = token;
}

void objetificar () {
    istringstream tokenizer {cod_obj};
    string token;

    while (tokenizer >> token) {
        output_line.push_back(stoi(token));
    }
}

void pega_nome () {
    istringstream tokenizer {input_line};
    string token;

    tokenizer >> token;
    tokenizer >> token;
        mod_name = token;
}

int main (int argc, char *argv[]) {
    ifstream input_file;
    ofstream output_file;

    if (argc > 1) {
        SOURCE_A_NAME = argv[1];
        input_file.open(SOURCE_A_NAME);
        getline (input_file, input_line);
        pega_nome ();
        getline (input_file, input_line);
        pega_fator_correcao ();
        getline (input_file, input_line);
        pega_info_reloc_A ();
        getline (input_file, input_line);
        while (input_line.front() == 'U') {
            input_line.erase(0,3);
            uso_A.push_back(input_line);
            getline (input_file, input_line);
        } while (input_line.front() == 'D') {
            input_line.erase(0,3);
            pega_tg_def_A ();
            getline (input_file, input_line);
        } input_line.erase(0,3);
        cod_obj.append(input_line+" ");
        input_file.close();

        if (argc > 2) {
            SOURCE_B_NAME = argv[2];
            input_file.open(SOURCE_B_NAME);
            getline (input_file, input_line);
            getline (input_file, input_line);
            getline (input_file, input_line);
            pega_info_reloc_B ();
            getline (input_file, input_line);
            while (input_line.front() == 'U') {
                input_line.erase(0,3);
                uso_B.push_back(input_line);
                getline (input_file, input_line);
            } while (input_line.front() == 'D') {
                input_line.erase(0,3);
                pega_tg_def_B ();
                getline (input_file, input_line);
            } input_line.erase(0,3);
            cod_obj.append(input_line);
            input_file.close();

            objetificar ();

            while (!uso_A.empty())
                insere_valores_finais_A ();
            insere_valores_finais_B ();

            for (unsigned int i=0; i<info_reloc_A.size(); i++) {
                if (info_reloc_A[i] == '1')
                    output_line[i] += fator_correcao_A;
            } for (unsigned int i=0; i<info_reloc_B.size(); i++) {
                if (info_reloc_B[i] == '1') {
                    output_line[i+fator_correcao_B] += fator_correcao_B;
                }
            }
        } else objetificar();
    } input_file.close();

            output_file.open(mod_name);
            for (unsigned int i=0; i < output_line.size(); i++)
                output_file << output_line[i] << " ";
            output_file.close();
    return 0;
}