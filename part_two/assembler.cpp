// ----------------------------------------------------------------------------------------------------
//    MONTADOR
// ----------------------------------------------------------------------------------------------------

using namespace std;
#include "filenames.hpp"
#include "instructions.hpp"
#include "tools.hpp"

// realiza a passagem unica
void singlepass (ifstream *input_file, ofstream *output_file, string *PREP_M_NAME, string *OBJECT_M_NAME) {
    input_file->open (*PREP_M_NAME);                // abre o arquivo pre-processado
    while ( !input_file->eof() ) {                  // enquanto arquivo nao acabou
        getline (*input_file, input_line);          // pega uma linha
        line_singlepass();                          // realiza a passagem unica da linha
    } input_file->close();                          // fecha o arquivo ao final da passagem
    write_object_file (output_file, OBJECT_M_NAME); // escreve o arquivo objeto
}

// realiza o pre-processamento
int preprocessing (ifstream *input_file, ofstream *output_file, string *SOURCE_M_NAME, string *PREP_M_NAME) {
    input_file->open (*SOURCE_M_NAME);                      // abre o arquivo fonte
    if ( input_file->is_open() ) {                          // se arquivo existe e esta aberto
        while ( !input_file->eof() ) {                      // enquanto arquivo nao acabou
            getline (*input_file, input_line);              // le linha do codigo fonte
            for (auto & c: input_line) c = toupper(c);      // retira sensibilidade ao caso
            if (!line_preprocessing (SOURCE_M_NAME))        // realiza o pre-processamento da linha
                break;                                      // diretiva END termina pre-processamento
        } input_file->close();                              // fecha arquivo fonte ao final do pre-processamento
        check_module (SOURCE_M_NAME);                       // organiza o arquivo em um modulo
        write_preprocessed_file (output_file, PREP_M_NAME); // escreve o arquivo pre-processado
    } else {
        cout << endl << "ERRO: arquivo \"" << *SOURCE_M_NAME << "\" nao encontrado!" << endl;
        return 0;   // retorna "arquivo nao existe"
    } return 1;     // retorna "arquivo existe"
}

// realiza o controle do processo de montagem
void assemble () {
    ifstream input_file;    // arquivo de entrada
    ofstream output_file;   // arquivo de saida

    // modulo A
    if (preprocessing (&input_file, &output_file, &SOURCE_A_NAME, &PREP_A_NAME))
        singlepass (&input_file, &output_file, &PREP_A_NAME, &OBJECT_A_NAME);

    // modulo B
    if (NUMBER_OF_FILES == 2)
        if (preprocessing (&input_file, &output_file, &SOURCE_B_NAME, &PREP_B_NAME))
            singlepass (&input_file, &output_file, &PREP_B_NAME, &OBJECT_B_NAME);
}

// inicia o programa principal
int main (int argc, char *argv[]) {
    cout << endl << ".::| MONTADOR MACCHIAVELLICO 2.0 |::." << endl;
    cout << endl << "Algumas caracteristicas bem especificas deste montador:" << endl;
    cout << "- Esta versao do montador NAO FAZ TESTE DE ERROS (obs.: com excecao de diretivas modulares)" << endl;
    cout << "- Diretiva END interrompe automaticamente a leitura de um arquivo fonte (obs.: para mais de um arquivo de entrada)" << endl;
    cout << "- Entrada: arquivos de texto com extensao \".asm\"" << endl;
    cout << "- O primeiro arquivo inserido sera tido como PRIMEIRO MODULO ou MODULO PRINCIPAL" << endl;
    cout << "- Saida: arquivos pre-processados com extensao \".pre\" e arquivos objeto \".obj\" para entrada no ligador apropriado" << endl;

    assign_source_filenames(argc, argv);    // recebe nomes dos arquivos de entrada
    if (NUMBER_OF_FILES > 0) {              // se os nomes dos arquivos forem validos
        stringSwitch();                     // inicializa palavras reservadas
        assign_output_filenames();          // atribui nomes dos arquivos de saida
        assemble();                         // poe a mao na massa com sangue nos olhos
    } else return 0;

    return 0;
}
