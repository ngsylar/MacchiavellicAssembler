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

// move SECTION DATA para baixo de SECTION TEXT
int move_section_DATA (unsigned int i) {    
    output_line[i] = "SECTION DATA";
    output_line.push_back (newline);

    while (i < output_line.size()) {
        output_line.push_back ( output_line[i] );

        if (output_line[i+1] != "SECTION") {
            i++;
        } else {
            if (output_line.back() == newline)
                output_line.pop_back();
            break;
        }
    }
    return i;
}

// escreve um token por vez
void write_token (ofstream *output_file, int i) {
    *output_file << output_line[i++];
    if ((i < output_line.size()) && (output_line[i] != newline))
        *output_file << " ";
}

// escreve o codigo pre-processado
void write_preprocessed_file (ofstream *output_file) {
    bool text_sign = false;     // sinaliza se escreveu SECTION TEXT
    // bool moved_data = false;    // sinaliza se moveu SECTION DATA

    while (output_line.back() == newline) {
        output_line.pop_back();
    }

    for (unsigned int i=0; i < output_line.size(); i++) {
        switch ( DIRECTIVE[ output_line[i] ] ) {

            case D_SECTION:
                if (!text_sign) switch ( SECTION[ output_line[++i] ] ) {

                    case S_TEXT:
                        *output_file << "SECTION TEXT";
                        text_sign = true;
                        break;

                    case S_DATA:
                        i = move_section_DATA (i);
                        // moved_data = true;
                        break;

                    default: break;
                } else write_token (output_file, i);
                break;

            default:
                if (output_line[i] == newline)
                    *output_file << newline;
                else
                    write_token (output_file, i);
                break;
        }
    }
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
