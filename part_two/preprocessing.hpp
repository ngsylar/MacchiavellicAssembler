// ----------------------------------------------------------------------------------------------------
//    PRE-PROCESSAMENTO
// ----------------------------------------------------------------------------------------------------

#ifndef _H_PREPROCESSING_
#define _H_PREPROCESSING_

// verifica a existencia de diretiva END ao final do programa
void check_directive_END (string *FILE_NAME) {
    if ((NUMBER_OF_FILES == 2) && (cursor.end_count == 0))
        error_handling (FILE_NAME, "NULL", 3);
}

// move SECTION DATA para baixo de SECTION TEXT
int move_section_DATA (unsigned int i) {
    output_line[i] = "SECTION DATA";
    output_line.push_back (newline);

    while (i < output_line.size()) {                // enquanto linha de saida nao acabou
        output_line.push_back ( output_line[i] );   // move token para o final da linha de saida

        if (output_line[i+1] != "SECTION") {        // se proximo token for diferente de SECTION
            i++;                                    // continua no laco
        } else {                                    // se for igual a SECTION, sai do loop
            if ((NUMBER_OF_FILES == 1) && (output_line.back() == newline))
                output_line.pop_back();
            break;
        }
    } return i;                                     // retorna posicao atual na lista de saida
}

// escreve um token por vez
void write_token (ofstream *output_file, unsigned int i) {
    *output_file << output_line[i++];   // escreve token atual e pega proximo token
    if ((i < output_line.size()) && (output_line[i] != newline))
        *output_file << " ";            // se proximo token nao for quebra de linha, escreve espaco
}

// escreve o codigo pre-processado
void write_preprocessed_file (ofstream *output_file, string *PREP_M_NAME) {
    bool text_sign = false;             // sinaliza se escreveu SECTION TEXT
    bool moved_data = false;            // sinaliza se moveu SECTION DATA
    output_file->open (*PREP_M_NAME);

    // retira quebras de linha do inicio e do final do arquivo
    while (output_line.front() == newline)
        output_line.erase (output_line.begin());
    while (output_line.back() == newline)
        output_line.pop_back();

    // enquanto linha de saida nao acabou
    for (unsigned int i=0; i < output_line.size(); i++) {
        switch ( DIRECTIVE[ output_line[i] ] ) {

            // se achou diretiva SECTION
            case D_SECTION:     // se ainda nao achou secao TEXT
                if (!text_sign && (i+1 != output_line.size())) switch ( SECTION[ output_line[++i] ] ) {

                    case S_TEXT:                        // se achou secao TEXT
                        *output_file << "SECTION TEXT"; // escreve no arquivo de saida
                        text_sign = true;               // sinaliza "achou secao TEXT"
                        break;

                    case S_DATA:                        // se achou secao DATA
                        i = move_section_DATA (i);      // move secao DATA para baixo de secao TEXT
                        moved_data = true;              // sinaliza "moveu secao DATA"
                        break;

                    default: break;
                } else {                            // se ja achou secao TEXT
                    write_token (output_file, i);   // escreve diretiva SECTION normalmente
                } break;
            
            // se achou diretiva END
            case D_END:
                if ((NUMBER_OF_FILES == 2) && (moved_data)) {           // se ha dois arquivos e secao DATA foi movida
                    if (output_line[i-1] == newline)                    // se token anterior for quebra de linha
                        output_line.erase (output_line.begin()+i-1);    // apaga quebra de linha
                    output_line.push_back("END");                       // move END para o novo final do arquivo
                    moved_data = 0;
                } else                              // caso contrario
                    write_token (output_file, i);   // escreve token normalmente
                break;

            // se achou outro tipo de token, escreve no arquivo de saida
            default:
                if (output_line[i] == newline)
                    *output_file << newline;
                else
                    write_token (output_file, i);
                break;
        }
    } // limpeza de variaveis
    output_file->close();
    output_line.clear();
    cursor.clear();
}

// remove comentarios
int clear_comment (string* token) {
    if (token->front() == ';') {    // se token for comentario
        // se nao estiver na primeira linha e nao houver quebra de linha precedente, insere qubra de linha
        if ((line_number != 1) && (output_line.back() != newline))
            output_line.push_back (newline);
        return 1;                   // retorna "achou comentario"
    } return 0;                     // retorna "nao achou comentario"
}

// verifica a existencia de rotulo nas linhas anteriores
int previous_label_is_not_empty (string token) {
    if ( !previous_label.empty() ) {    // se houver rotulo na linha anterior
        // se arquivo for unico ou diretiva BEGIN ja foi declarada
        if ((NUMBER_OF_FILES == 1) || (cursor.begin_count > 0))
            return 1;                   // retorna "achou rotulo"
        else if (token != "BEGIN")      // caso contrario, se token atual for diferente de BEGIN
            previous_label.clear();     // limpa rotulos encontrados e retorna "nao achou rotulo"
        else return 1;                  // se token atual for diretiva BEGIN, retorna "achou rotulo"
    } return 0;                         // se nao houver rotulo na linha anterior, retorna "nao achou rotulo"
}

// inicia o pre-processamento
int line_preprocessing (string *FILE_NAME) {
    istringstream tokenizer {input_line};   // decompositor de linha
    string token;                           // string lida na linha de entrada
    line_number++;

    if (tokenizer >> token) { do {      // enquanto linha nao acabou, pega um token
        if (clear_comment (&token))     // se token for comentario, apagar
            return 1;                   // e pular para a proxima linha

        if (token.back() == ':') {              // se token for rotulo
            previous_label.push_back (token);   // guarda o rotulo em uma variavel temporaria
            if (tokenizer >> token) do {                            // pega o proximo token da linha
                if (token.back() == ':') {                          // se token for rotulo
                    for (unsigned int i=0; i < previous_label.size(); i++) {
                        if (token == previous_label[i]) break;      // nao guarda caso seja igual ao anterior
                        else if ((i+1) == previous_label.size())    // se for dirente
                            previous_label.push_back (token);       // guarda na variavel temporaria
                    }
                } else if (token.front() == ';') {                  // se token for comentario ou fim da linha
                    return 1;                                       // pula para proxima linha
                } else break;                                       // se token for de outro tipo, sai do laco
            } while (tokenizer >> token);
            else return 1;                                          // se linha acabou, pula para proxima
        }

        if ( previous_label_is_not_empty (token) ) {        // se existe rotulo anterior
            for (unsigned int i=0; i < previous_label.size(); i++)
                output_line.push_back (previous_label[i]);  // insere rotulo na linha de saida
            previous_label.clear();                         // limpa variavel temporaria
        }

        if ((NUMBER_OF_FILES == 1) || (cursor.begin_count > 0))
            output_line.push_back(token);   // insere token na linha de saida

        // se token for diretiva
        switch ( DIRECTIVE[token] ) {
            case D_BEGIN:                                       // DIRETIVA BEGIN
                if (NUMBER_OF_FILES == 1) {                     // se ha apenas um modulo
                    error_handling (FILE_NAME, token, 1);       // erro
                }
                else {                                          // se ha mais de um modulo
                    output_line.push_back(token);               // escreve diretiva BEGIN na linha de saida
                    cursor.begin_count++;                       // incrementa contagem de diretiva de modulo
                    if (cursor.begin_count > 1)                 // se houver redeclaracao de diretiva
                        error_handling (FILE_NAME, token, 2);   // erro
                }
                output_line.push_back (newline);                // insere quebra de linha na linha de saida
                return 1;                                       // pular para proxima linha

            case D_END:                                         // DIRETIVA END
                if (NUMBER_OF_FILES == 1) {                     // se ha apenas um modulo
                    error_handling (FILE_NAME, token, 1);       // erro
                }
                else {                                          // se ha mais de um modulo
                    cursor.end_count++;                         // incrementa contagem de diretiva de modulo
                    if (cursor.begin_count == 0)                // se nao houve declaracao previa de BEGIN
                        error_handling (FILE_NAME, NULL, 4);    // erro
                    return 0;                                   // terminar pre-processamento
                }
                output_line.push_back (newline);                // insere quebra de linha na linha de saida
                return 1;                                       // pular para proxima linha

            case D_PUBLIC:                                  // DIRETIVA PUBLIC
                if (NUMBER_OF_FILES == 1)                   // se ha apenas um modulo
                    error_handling (FILE_NAME, token, 1);   // erro
                break;
            
            case D_EXTERN:                                  // DIRETIVA EXTERN
                if (NUMBER_OF_FILES == 1)                   // se ha apenas um modulo
                    error_handling (FILE_NAME, token, 1);   // erro
                break;

            default: break;
        }
    } while (tokenizer >> token);
    output_line.push_back (newline); }  // insere quebra de linha na linha de saida
    return 1;                           // pular para leitura da proxima linha no arquivo fonte
}

#endif
