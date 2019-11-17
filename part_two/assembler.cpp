// ----------------------------------------------------------------------------------------------------
//    MONTADOR
// ----------------------------------------------------------------------------------------------------

using namespace std;
#include "filenames.hpp"
#include "instructions.hpp"
#include "tools.hpp"

// inicia processo de montagem
void assemble () {
    ifstream input_file (SOURCE_A_NAME);

    if ( input_file.is_open() ) {
        while ( !input_file.eof() ) {                   // enquanto arquivo nao acabou
            getline (input_file, input_line);           // le linha do codigo fonte
            for (auto & c: input_line) c = toupper(c);  // retira sensibilidade ao caso
            preprocessing();                            // realiza o pre-processamento
        } input_file.close();
    }
}

// programa principal
int main (int argc, char *argv[]) {
    assign_source_filenames(argc, argv);    // recebe nomes dos arquivos de entrada

    if (NUMBER_OF_FILES > 0) {      // se os nomes dos arquivos forem validos
        stringSwitch();             // inicializa palavras reservadas
        assign_output_filenames();  // atribui nomes dos arquivos de saida
        assemble();
    } else return 0;

    cout << output_line.size() << endl;
    for (int i=0; i < output_line.size(); i++) cout << output_line[i];
    return 0;
}
