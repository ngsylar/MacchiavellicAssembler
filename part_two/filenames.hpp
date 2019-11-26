// ----------------------------------------------------------------------------------------------------
//    NOMES DE ARQUIVOS
// ----------------------------------------------------------------------------------------------------

#ifndef _H_FILENAMES_
#define _H_FILENAMES_

// bibliotecas
#include <iostream>

// constantes tipo ascii
#define ASCII_LINEFEED 0x0A
#define ASCII_SPACE 0x20
#define ASCII_DOT 0x2E

// constantes mnemonicas
#define SOURCE_A_NAME SOURCE_NAME[0]
#define SOURCE_B_NAME SOURCE_NAME[1]
#define PREP_A_NAME PREP_NAME[0]
#define PREP_B_NAME PREP_NAME[1]
#define OBJECT_A_NAME OBJECT_NAME[0]
#define OBJECT_B_NAME OBJECT_NAME[1]

// constantes simbolicas
static int NUMBER_OF_FILES = 0;
static string SOURCE_NAME[2], PREP_NAME[2], OBJECT_NAME[2];

// recebe nomes dos arquivos de entrada
void assign_source_filenames (int argc, char *argv[]) {
    string extension;

    if (argc > 1) {                 // se inicio recebeu mais de um argumento
        SOURCE_A_NAME = argv[1];    // nome do primeiro modulo recebe segundo argumento

        if (argc > 2) {                 // se existe mais argumentos
            SOURCE_B_NAME = argv[2];    // nome do terceiro modulo recebe terceiro argumento
        }
    } else {                                        // se inicio so recebeu um argumento
        SOURCE_A_NAME = "used_tests\\modules\\test1_A.asm";
        SOURCE_B_NAME = "used_tests\\modules\\test1_B.asm";
        // cout << "Insira o nome dos arquivos: ";     // pede para usuario inserir nome dos arquivos
        // cin >> SOURCE_A_NAME;                       // modulo A recebe nome do buffer

        // try {
        //     throw getchar();                        // recebe um caractere do buffer
        // } catch (int next_char) {                   // se nao for quebra de linha
        //     while (next_char == ASCII_SPACE)        // se for espaco
        //         next_char = getchar();              // ignora todos os espacos
        //     // se achar caractere diferente de quebra de linha
        //     while ((next_char != ASCII_LINEFEED) && (next_char != ASCII_SPACE)) {
        //         SOURCE_B_NAME += (char)next_char;   // modulo B recebe nome do buffer
        //         next_char = getchar();              // caractere a caractere, ate encontrar quebra de linha ou espaco
        //     }
        // }
    } setbuf (stdin, NULL);     // limpar demais argumentos, se existirem

    // confere extensao dos arquivos
    try {
        for (int i = SOURCE_A_NAME.size() - 1; i >= 0; i--) {
            extension.insert (0, 1, SOURCE_A_NAME[i]);  // pega caracteres do final do nome do modulo A
            if (SOURCE_A_NAME[i] == ASCII_DOT) {        // se achar ponto
                if (extension == ".asm") {              // se extensao for .asm
                    if (SOURCE_B_NAME.empty()) {        // se nao existe modulo B
                        throw 0;                        // nao ha erro
                    } else {                                            // se existe modulo B
                        extension.clear();
                        for (int j = SOURCE_B_NAME.size() - 1; j >= 0; j--) {
                            extension.insert (0, 1, SOURCE_B_NAME[j]);  // pega caracteres do final
                            if (SOURCE_B_NAME[j] == ASCII_DOT) {        // se achar ponto
                                if (extension == ".asm") throw 0;       // se extensao for .asm, nao ha erro
                                else throw 1;                           // se extensao for diferente, erro
                            }
                        } throw 1;                                      // se nao achou ponto, erro
                    }
                } else throw 1;                         // se extensao for diferente, erro
            }
        } throw 1;                                      // se nao achou ponto, erro
    } catch (int invalid_extension) {
        if (invalid_extension) {
            cout << endl << "ERRO: formato de arquivo invalido!" << endl;
            return;
        }
    }
    // atribui numero de arquivos recebidos
    if ( SOURCE_B_NAME.empty() )
        NUMBER_OF_FILES = 1;
    else
        NUMBER_OF_FILES = 2;
}

// atribui nomes dos arquivos de saida
void assign_output_filenames () {
    // modulo A
    PREP_A_NAME.append (SOURCE_A_NAME, 0, SOURCE_A_NAME.size()-3);
    OBJECT_A_NAME.append (PREP_A_NAME);
    PREP_A_NAME.append("pre");
    OBJECT_A_NAME.append("obj");

    if (NUMBER_OF_FILES == 2) {
        PREP_B_NAME.append (SOURCE_B_NAME, 0, SOURCE_B_NAME.size()-3);
        OBJECT_B_NAME.append (PREP_B_NAME);
        PREP_B_NAME.append("pre");
        OBJECT_B_NAME.append("obj");
    }
    // cout << SOURCE_A_NAME << " " << PREP_A_NAME << " " << OBJECT_A_NAME << " ";
}

#endif
