// ----------------------------------------------------------------------------------------------------
//    BIBLIOTECAS
// ----------------------------------------------------------------------------------------------------

#include <iostream>
using namespace std;

// ----------------------------------------------------------------------------------------------------
//    CONSTANTES
// ----------------------------------------------------------------------------------------------------

#define ASCII_LINEFEED 0x0A
#define ASCII_SPACE 0x20
#define ASCII_DOT 0x2E

#define SOURCE_A_NAME SOURCE_NAME[0]
#define SOURCE_B_NAME SOURCE_NAME[1]

// ----------------------------------------------------------------------------------------------------
//    VARIAVEIS GLOBAIS
// ----------------------------------------------------------------------------------------------------

static int NUMBER_OF_FILES = 0;
static string* SOURCE_NAME;

// ----------------------------------------------------------------------------------------------------
//    NOMES DE ARQUIVOS
// ----------------------------------------------------------------------------------------------------

// recebe nome dos arquivos de entrada
void allocate_source_filenames (int argc, char *argv[]) {
    string source_A, source_B, extension;

    if (argc > 1) {             // se inicio recebeu mais de um argumento
        source_A = argv[1];     // nome do primeiro modulo recebe segundo argumento

        if (argc > 2) {         // se existe mais argumentos
            source_B = argv[2]; // nome do terceiro modulo recebe terceiro argumento
        }
    } else {                                        // se inicio so recebeu um argumento
        cout << "Insira o nome dos arquivos: ";     // pede para usuario inserir nome dos arquivos
        cin >> source_A;                            // modulo A recebe nome do buffer

        try {
            throw getchar();                    // recebe um caractere do buffer
        } catch (int next_char) {               // se nao for quebra de linha
            while (next_char == ASCII_SPACE)    // se for espaco
                next_char = getchar();          // ignora todos os espacos
            // se achar caractere diferente de quebra de linha
            while ((next_char != ASCII_LINEFEED) && (next_char != ASCII_SPACE)) {
                source_B += (char)next_char;    // modulo B recebe nome do buffer
                next_char = getchar();          // caractere a caractere, ate encontrar quebra de linha ou espaco
            }
        }
    } setbuf (stdin, NULL);             // limpar demais argumentos, se existirem

    // confere extensao dos arquivos
    try {
        for (int i = source_A.size() - 1; i >= 0; i--) {
            extension.insert (0, 1, source_A[i]);   // pega caracteres do final do nome do modulo A
            if (source_A[i] == ASCII_DOT) {         // se achar ponto
                if (extension == ".asm") {          // se extensao for .asm
                    if (source_B.empty()) {         // se nao existe modulo B
                        throw 0;                    // nao ha erro
                    } else {                                        // se existe modulo B
                        extension.clear();
                        for (int j = source_B.size() - 1; j >= 0; j--) {
                            extension.insert (0, 1, source_B[j]);   // pega caracteres do final
                            if (source_B[j] == ASCII_DOT) {         // se achar ponto
                                if (extension == ".asm") throw 0;   // se extensao for .asm, nao ha erro
                                else throw 1;                       // se extensao for diferente, erro
                            }
                        } throw 1;                                  // se nao achou ponto, erro
                    }
                } else throw 1;                     // se extensao for diferente, erro
            }
        } throw 1;                                  // se nao achou ponto, erro
    } catch (int invalid_extension) {
        if (invalid_extension) {
            cout << endl << "ERRO: formato de arquivo invalido!" << endl;
            return;
        }
    }

    if (source_B.empty()) {             // se nao existe modulo B
        SOURCE_NAME = new string;       // aloca um espaco de memoria para nome de arquivo
        *SOURCE_NAME = source_A;        // ponteiro recebe apenas nome do modulo A
        NUMBER_OF_FILES = 1;            // define numero de arquivos igual a um
    }
    else {                              // se existe modulo B
        SOURCE_NAME = new string[2];    // aloca dois espacos de memoria para nome de arquivo
        SOURCE_A_NAME = source_A;       // ponteiro recebe nome dos modulos A e B
        SOURCE_B_NAME = source_B;
        NUMBER_OF_FILES = 2;            // define numero de arquivos igual a dois
    }
}

// desaloca nome dos arquivos de entrada
void deallocate_source_filenames () {
    if (NUMBER_OF_FILES == 2)
        delete[] SOURCE_NAME;
    else if (NUMBER_OF_FILES == 1)
        delete SOURCE_NAME;
}

// ----------------------------------------------------------------------------------------------------
//    MONTADOR
// ----------------------------------------------------------------------------------------------------

// inicia execucao do montador
int main (int argc, char *argv[]) {
    // stringSwitch();     // inicializa palavras reservadas
    allocate_source_filenames(argc, argv);

    deallocate_source_filenames();
    return 0;
}
