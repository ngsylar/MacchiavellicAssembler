// ----------------------------------------------------------------------------------------------------
//    PRE-PROCESSAMENTO
// ----------------------------------------------------------------------------------------------------

#ifndef _H_PREPROCESSING_
#define _H_PREPROCESSING_

// inicia o pre-processamento
void preprocessing () {
    istringstream tokenizer {input_line};   // decompositor de linha
    string token;                           // string lida na linha de entrada
    line_number++;

    while (tokenizer >> token) {        // enquanto linha nao acabou, pega um token
        output_line.push_back(token);   // insere token na lina de saida

        // se token for diretiva SECTION, verificar localizacao e contagem
        switch ( DIRECTIVE[token] ) {
            case D_SECTION:
                tokenizer >> token;
                output_line.push_back(token);
                output_line.push_back(newline);
                return;
            default: break;
        }

        // // se token for identificador
        // if (token.back() == ':') {
        //     token.pop_back();                                   // descarta ':'
        //     word.insert(token);                                 // forca inicializacao de word.content(): passa token para word.aux
        //     mtl = word.multiple_labels (&tokenizer, &token);    // verifica se ha mais de um identificador na mesma linha
        //     clear_comment (&tokenizer, &token, true);           // limpa comentario, se existir, apos identificador
        //     if ( tokenizer.eof() ) {                            // se houver quebra de linha apos identificador
        //         token_aux = word.content();                     // guarda identificador em token_aux
        //     }
        // }
        // // se nao houver identificador na linha atual, verifica a existencia de identificador na linha anterior
        // else if ( !token_aux.empty() ) {
        //     word.insert(token_aux);
        //     token_aux.clear();
        // }

        // // analisa o token (o token seguinte ao identificador, se existir identificador)
        // switch ( DIRECTIVE[ token ] ) {
        //     case d_EQU:

        //         // se diretiva EQU aparece apos secoes declaradas, erro
        //         if ( cursor.got_in() ) {
        //             std::cout << endl << "Linha " << line_number << " de [" << *file_name << "]:" << endl;
        //             std::cout << "Erro Semantico: diretiva EQU apos declaracao de secao" << endl;
        //         }

        //         // se identificador nao estiver vazio e nao for repetido, inserir numa tabela relacionando-o com seu sinonimo
        //         if ( !word.empty() ) {
        //             if (word.check_label (file_name, token, mtl)) {     // analisa validade do identificador
        //                 // se linha nao acabou, pega um numero se token nao for comentario
        //                 if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false)) {
        //                     word.check_number (file_name, token);           // analisa validade do numero
        //                     if (ident_table.search (word.content()) ) {     // se identificador ja estava na tabela, erro
        //                         std::cout << endl << "Linha " << line_number << " de [" << *file_name << "]:" << endl;
        //                         std::cout << "Erro Semantico: rotulo \"" << word.content() << "\" ja foi declarado" << endl;
        //                     } else                                          // se identificador nao esta na tabela
        //                         ident_table.insert (word.content(), token); // insere identificador na tabela
        //                 } else {                                            // se linha acabou, erro
        //                     std::cout << endl << "Linha " << line_number << " de [" << *file_name << "]:" << endl;
        //                     std::cout << "Erro Sintatico: constante ausente em declaracao EQU" << endl;
        //                 }
        //             } else while (tokenizer >> token);                  // se identificador for invalido, ignorar linha
        //             clear_EQU_line (&tokenizer, &token, word.empty());  // remove diretiva EQU da linha de saida
        //             word.clear();                                       // limpa identificador para proxima linha
        //         }

        //         // se identificador estiver vazio, erro na diretiva EQU
        //         else {
        //             std::cout << endl << "Linha " << line_number << " de [" << *file_name << "]:" << endl;
        //             std::cout << "Erro Sintatico: rotulo para diretiva EQU nao existe" << endl;
        //             clear_EQU_line (&tokenizer, &token, word.empty());  // remove diretiva EQU da linha de saida
        //         }

        //         // se linha nao acabou, captura proximo token, se token for comentario, pula linha, se nao erro
        //         if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false)) {
        //             std::cout << endl << "Linha " << line_number << " de [" << *file_name << "]:" << endl;
        //             std::cout << "Erro Sintatico: diretiva EQU recebendo muitos argumentos" << endl;
        //             while (tokenizer >> token);     // ignora o restante da linha
        //         }
        //         break;

        //     case d_IF:

        //         // se linha nao acabou, pega proximo token, se token for comentario, pular linha
        //         if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false))
        //             word.check_token (file_name, token);    // se nao for comentario, analisa token
        //         else {                                      // se nao ha identificador ou constante, erro
        //             std::cout << endl << "Linha " << line_number << " de [" << *file_name << "]:" << endl;
        //             std::cout << "Erro Sintatico: argumento ausente em declaracao IF" << endl;
        //             outline.pop_back();
        //         }

        //         // se linha nao acabou, pega proximo token, se token for comentario, pula linha, se nao erro
        //         if (!tokenizer.eof() && (tokenizer >> token) && !clear_comment (&tokenizer, &token, false)) {
        //             std::cout << endl << "Linha " << line_number << " de [" << *file_name << "]:" << endl;
        //             std::cout << "Erro Sintatico: diretiva IF recebendo muitos argumentos" << endl;
        //             while (tokenizer >> token);     // ignora o restante da linha
        //         }
        //         break;

        //     default:
        //         // se token for comentario, pular para proxima linha
        //         clear_comment (&tokenizer, &token, true);
        //         break;
        // }
    }
    output_line.push_back (newline);    // insere quebra de linha na linha de saida
}

#endif
