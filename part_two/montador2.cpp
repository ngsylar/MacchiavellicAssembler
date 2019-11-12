// ----------------------------------------------------------------------------------------------------
//    BIBLIOTECAS
// ----------------------------------------------------------------------------------------------------

#include <iostream>
using namespace std;

// ----------------------------------------------------------------------------------------------------
//    CONSTANTES GLOBAIS TIPO ASCII
// ----------------------------------------------------------------------------------------------------

#define ASCII_LINEFEED 0x0A
#define ASCII_SPACE 0x20

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
    string source_A, source_B;

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

    // nota: conferir se nomes tem extensao .asm

    if (source_B.empty()) {             // se nao existe modulo B
        SOURCE_NAME = new string;       // aloca um espaco de memoria para nome de arquivo
        *SOURCE_NAME = source_A;        // ponteiro recebe apenas nome do modulo A
        NUMBER_OF_FILES = 1;            // define numero de arquivos igual a um
    }
    else {                              // se existe modulo B
        SOURCE_NAME = new string[2];    // aloca dois espacos de memoria para nome de arquivo
        SOURCE_NAME[0] = source_A;      // ponteiro recebe nome dos modulos A e B
        SOURCE_NAME[1] = source_B;
        NUMBER_OF_FILES = 2;            // define numero de arquivos igual a dois
    }
}

// desaloca nome dos arquivos de entrada
void deallocate_source_filenames () {
    if (NUMBER_OF_FILES == 2)
        delete[] SOURCE_NAME;
    else
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
