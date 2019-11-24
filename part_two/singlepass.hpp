// ----------------------------------------------------------------------------------------------------
//    SINTESE DE CODIGO
// ----------------------------------------------------------------------------------------------------

#ifndef _H_SINGLE_PASS_
#define _H_SINGLE_PASS_

void treat_labels (istringstream *tokenizer, string *token) {
    vector<string> label;
    token->pop_back();                   // ignora ':'
    label.push_back (*token);

    while (*tokenizer >> *token)
        if (token->back() == ':') {
            token->pop_back();
            label.push_back (*token);
        } else if (*token == "EXTERN") {
            for (unsigned int i=0; i < label.size(); i++)
                symbol.insert_external (label[i]);
            return;
        } else {
            for (unsigned int i=0; i < label.size(); i++)
                symbol.insert_defined (label[i], program_address);
                // nota: aqui ainda precisa atualizar linhas anteriores onde o rotulo aparece
            break;
        }
}

void line_singlepass (string *FILE_NAME) {
    istringstream tokenizer {input_line};
    string token;

    while (tokenizer >> token) {
        if (token.back() == ':')
            treat_labels (&tokenizer, &token);
        
        // nota: aqui comeca o tratamento dos outros tokens
    }
}

#endif
