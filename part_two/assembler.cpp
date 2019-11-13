// ----------------------------------------------------------------------------------------------------
//    MONTADOR
// ----------------------------------------------------------------------------------------------------

// bibliotecas
#include "instructions.hpp"
#include "files.hpp"

// inicia execucao do montador
int main (int argc, char *argv[]) {
    allocate_source_filenames(argc, argv);

    if (NUMBER_OF_FILES > 0) {
        stringSwitch();     // inicializa palavras reservadas
        cout << OPCODE["DIV"];
        if (NUMBER_OF_FILES == 1) {
            ifstream SOURCE_FILE (*SOURCE_NAME);
        }
    } else return 0;

    cout << *SOURCE_NAME;
    deallocate_source_filenames();
    cout << SOURCE_NAME->size();
    return 0;
}
