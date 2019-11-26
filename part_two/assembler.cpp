// ----------------------------------------------------------------------------------------------------
//    MONTADOR
// ----------------------------------------------------------------------------------------------------

using namespace std;
#include "filenames.hpp"
#include "instructions.hpp"
#include "tools.hpp"
unsigned int balada = 0;
// realiza a passagem unica
void singlepass (ifstream *input_file, ofstream *output_file, string *PREP_M_NAME, string *OBJECT_M_NAME) {
    input_file->open (*PREP_M_NAME);
    while ( !input_file->eof() ) {
        getline (*input_file, input_line);
        line_singlepass();
    } input_file->close();

    // nota: testes
    cout << endl << cursor.module_name[balada++] << endl;
    for (unsigned int i=0; i < output_code.size(); i++)
        cout << output_code[i] << " ";
    cout << endl;
    // for (unsigned int i=0; i < symbol.t_body.size(); i++)
    //     cout << endl << symbol.t_body[i].label << endl << "val " << symbol.t_body[i].value << "\tdef " << symbol.t_body[i].defined << "\tpub " << symbol.t_body[i].shared << "\text " << symbol.t_body[i].external << endl;
    output_code.clear();
    symbol.clear();
    program_address = 0;
}

// realiza o pre-processamento
int preprocessing (ifstream *input_file, ofstream *output_file, string *SOURCE_M_NAME, string *PREP_M_NAME) {
    input_file->open (*SOURCE_M_NAME);
    if ( input_file->is_open() ) {
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
    ifstream input_file;
    ofstream output_file;

    // modulo A
    if (preprocessing (&input_file, &output_file, &SOURCE_A_NAME, &PREP_A_NAME))
        singlepass (&input_file, &output_file, &PREP_A_NAME, &OBJECT_A_NAME);
    else return;

    // modulo B
    if (NUMBER_OF_FILES == 2)
        if (preprocessing (&input_file, &output_file, &SOURCE_B_NAME, &PREP_B_NAME))
            singlepass (&input_file, &output_file, &PREP_B_NAME, &OBJECT_B_NAME);
}

// inicia o programa principal
int main (int argc, char *argv[]) {
    cout << endl << "Algumas caracteristicas bem especificas deste montador:\n- Diretiva END interrompe automaticamente a leitura de um arquivo fonte (obs.: para mais de um arquivo de entrada)\n- Entrada: arquivos de texto com extensao \".asm\"\n- Saida: arquivos pre-processados com extensao \".pre\" e arquivos objeto \".sni\" para entrada no ligador" << endl;

    assign_source_filenames(argc, argv);    // recebe nomes dos arquivos de entrada
    if (NUMBER_OF_FILES > 0) {              // se os nomes dos arquivos forem validos
        stringSwitch();                     // inicializa palavras reservadas
        assign_output_filenames();          // atribui nomes dos arquivos de saida
        assemble();
    } else return 0;

    return 0;
}
