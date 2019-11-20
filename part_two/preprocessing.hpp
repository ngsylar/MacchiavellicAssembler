// ----------------------------------------------------------------------------------------------------
//    PRE-PROCESSAMENTO
// ----------------------------------------------------------------------------------------------------

#ifndef _H_PREPROCESSING_
#define _H_PREPROCESSING_

// verifica a existencia de diretiva END ao final do programa
void check_directive_END (string *FILE_NAME) {
    if ((NUMBER_OF_FILES == 2) && (cursor.d_end_count == 0))
        error_handling (FILE_NAME, NULL, 3);
}

void write_preprocessed_file (ofstream *output_file) {
}

// remove comentarios
int clear_comment (string* token) {
    if (token->front() == ';') {
        if (line_number != 1)                   // se leitura nao estiver na primeira linha do codigo
            output_line.push_back (newline);    // insere quebra de linha na linha de saida
        return 1;                               // retorna "achou comentario"
    } return 0;                                 // retorna "nao achou comentario"
}

// inicia o pre-processamento
int preprocessing (string *FILE_NAME) {
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
                } else break;
            } while (tokenizer >> token);
            else return 1;                                          // se linha acabou, pula para proxima
        }

        if ( !previous_label.empty() ) {                    // se existe rotulo anterior
            for (unsigned int i=0; i < previous_label.size(); i++)
                output_line.push_back (previous_label[i]);  // insere rotulo na linha de saida
            previous_label.clear();                         // limpa variavel temporaria
        }

        output_line.push_back(token);   // insere token na linha de saida

        // se token for diretiva
        switch ( DIRECTIVE[token] ) {
            case D_BEGIN:                                       // DIRETIVA BEGIN
                if (NUMBER_OF_FILES == 1) {                     // se ha apenas um modulo
                    error_handling (FILE_NAME, token, 1);       // erro
                }
                else {                                          // se ha mais de um modulo
                    cursor.d_begin_count++;                     // incrementa contagem de diretiva de modulo
                    if (cursor.d_begin_count > 1)               // se houver redeclaracao de diretiva
                        error_handling (FILE_NAME, token, 2);   // erro
                }
                output_line.push_back (newline);                // insere quebra de linha na linha de saida
                return 1;                                       // pular para proxima linha

            case D_END:                                         // DIRETIVA END
                if (NUMBER_OF_FILES == 1) {                     // se ha apenas um modulo
                    error_handling (FILE_NAME, token, 1);       // erro
                }
                else {                                          // se ha mais de um modulo
                    cursor.d_end_count++;                       // incrementa contagem de diretiva de modulo
                    if (cursor.d_begin_count == 0)              // se nao houve declaracao previa de BEGIN
                        error_handling (FILE_NAME, NULL, 4);    // erro
                    return 0;                                   // terminar pre-processamento
                }
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
