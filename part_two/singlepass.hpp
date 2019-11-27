// ----------------------------------------------------------------------------------------------------
//    PASSAGEM UNICA
// ----------------------------------------------------------------------------------------------------

#ifndef _H_SINGLEPASS_
#define _H_SINGLEPASS_

// escreve o codigo objeto nao ligado
void write_object_file (ofstream *output_file, string *FILE_NAME) {
    unsigned int module_index;              // guarda o indice do modulo atual
    string module_name;                     // guarda o nome do modulo atual
    Table usage_table, definitions_table;   // tabelas de uso e de definicao
    output_file->open (*FILE_NAME);

    // define nome e indice do modulo
    if (NUMBER_OF_FILES == 1)
        module_index = cursor.module_index-1;
    else
        module_index = cursor.module_index-2;
    module_name = cursor.module_name[ module_index ];
    
    // escreve cabecalho
    *output_file << "H: " << module_name;
    if (NUMBER_OF_FILES == 2)
        *output_file << " " << module_index;
    *output_file << endl << "H: " << output_code.size() << endl;
    *output_file << "H: informacao de realocacao aqui" << endl;

    // escreve tabelas de uso e definicoes
    if (NUMBER_OF_FILES == 2) {
        symbol.make_link_tables (&usage_table, &definitions_table);
        for (unsigned int i=0; i < usage_table.size(); i++)
            *output_file << "U: " << usage_table.mailing_list(i) << endl;
        for (unsigned int i=0; i < definitions_table.size(); i++)
            *output_file << "D: " << definitions_table.symbol_value(i) << endl;
    }

    // escreve codigo objeto
    *output_file << "T: ";
    for (unsigned int i=0; i < output_code.size(); i++) {
        *output_file << output_code[i];
        if (i+1 != output_code.size())
            *output_file << " ";
    }

    // limpeza de variaveis
    output_file->close();
    output_code.clear();
    symbol.clear();
    program_address = 0;
}

// classe para processamento de operacoes
class Processor {
    public:

    // faz o processamento de operandos
    void operands (Analyze *word, string token) {
        int value = word->check_argument (token, program_address);  // calcula valor da expressao
        output_code.push_back (value);                              // insere o resultado na linha de saida
        program_address++;                                          // incrementa endereco
    }

    // faz o processamento de uma operacao COPY
    void operation_COPY (Analyze *word, istringstream *tokenizer, string *token) {
        vector<string> argument;
        output_code.push_back(9);   // insere codigo da operacao na linha de saida
        program_address++;          // incrementa endereco

        if (*tokenizer >> *token)                           // pega a expressao seguinte ao codigo da operacao
            word->check_expression (*token, &argument);     // salva os argumentos
        for (unsigned int i=0; i < argument.size(); i++)    // para cada argumento
            operands (word, argument[i]);                   // processa os operandos
    }

    // faz o processamento de uma operacao
    void operation (Analyze *word, istringstream *tokenizer, string *token, int operation) {
        output_code.push_back (operation);  // insere codigo da operacao na linha de saida
        program_address++;                  // incrementa endereco
        if (*tokenizer >> *token)           // pega a expressao seguinte a operacao
            operands (word, *token);        // processa os operandos
    }
};

// atualiza valores de chamada anteriores a definicao de simbolo
void update_call_values () {
    vector<int> adresses;
    symbol.current_list (&adresses);                            // salva lista de enderecoes do simbolo atual em um vetor
    for (unsigned int i=0; i < adresses.size(); i++)            // para cada endereco salvo na lista
        output_code[ adresses[i] ] += symbol.current.value;     // atualizar o valor salvo no endereco
}

// trata rotulos
void treat_labels (istringstream *tokenizer, string *token) {
    vector<string> label;
    token->pop_back();          // token atual eh rotulo
    label.push_back (*token);   // salva numa lista

    while (*tokenizer >> *token)            // enquanto linha nao acabou, pega proximo token
        if (token->back() == ':') {         // se token for rotulo
            token->pop_back();              // ignora ':'
            label.push_back (*token);       // salva rotulo
        } else if (*token == "EXTERN") {                    // se token for diretiva EXTERN
            for (unsigned int i=0; i < label.size(); i++)
                symbol.insert_external (label[i]);          // declara rotulos salvos como simbolos externos
            return;                                         // volta para sintese do codigo
        } else {        // se token for de outro tipo, declara rotulos salvos como definidos
            for (unsigned int i=0; i < label.size(); i++) {
                if (symbol.insert_defined (label[i], program_address))
                    update_call_values();
            } break;
        }
}

// faz passagem unica para cada linha do arquivo pre-processado
void line_singlepass () {
    Analyze word;
    Processor process;
    istringstream tokenizer {input_line};
    string token;

    bool not_directive = false;
    while (tokenizer >> token) {                // enquanto linha nao acabou, pega um token
        if (token.back() == ':')                // se token for declaracao de rotulo
            treat_labels (&tokenizer, &token);  // define tipo do rotulo declarado
        
        // se token for diretiva
        switch ( DIRECTIVE[token] ) {

            case D_SPACE:                       // DIRETIVA SPACE
                if ((tokenizer >> token) && word.check_number (token, false)) {
                    for (int i=0; i < word.number; i++)     // pega proximo token e extrai o valor
                        output_code.push_back(0);           // reserva "valor" espacos na linha de saida
                    program_address += word.number;         // incrementa endereco em "valor" posicoes
                } else {                        // se nao foram passados argumentos
                    output_code.push_back(0);   // reserva apenas um espaco
                    program_address++;          // incrementa endereco
                } break;

            case D_CONST:                       // DIRETIVA CONST   
                if ((tokenizer >> token) && word.check_number (token, true)) {
                    output_code.push_back (word.number);    // pega proximo token e escreve o valor na linha de saida
                } else {                        // em caso de erro
                    output_code.push_back(0);   // escreve zero na linha de saida
                } program_address++;            // incrementa endereco
                break;

            case D_PUBLIC:                          // DIRETIVA PUBLIC
                if (tokenizer >> token)             // pega protimo token
                    symbol.insert_public (token);   // insere na tabela de simbolos como publico
                break;

            default:                    // caso token nao seja diretiva
                not_directive = true;   // vai para processamento de operacoes
                break;
        }

        // se token for opcode
        if (not_directive) switch ( OPCODE[token] ) {

            // expressoes aritmeticas
            case ADD:
                process.operation (&word, &tokenizer, &token, 1);
                break;
            case SUB:
                process.operation (&word, &tokenizer, &token, 2);
                break;
            case MULT:
                process.operation (&word, &tokenizer, &token, 3);
                break;
            case DIV:
                process.operation (&word, &tokenizer, &token, 4);
                break;

            // saltos
            case JMP:
                process.operation (&word, &tokenizer, &token, 5);
                break;
            case JMPN:
                process.operation (&word, &tokenizer, &token, 6);
                break;
            case JMPP:
                process.operation (&word, &tokenizer, &token, 7);
                break;
            case JMPZ:
                process.operation (&word, &tokenizer, &token, 8);
                break;

            // copia
            case COPY:
                process.operation_COPY (&word, &tokenizer, &token);
                break;

            // memoria
            case LOAD:
                process.operation (&word, &tokenizer, &token, 10);
                break;
            case STORE:
                process.operation (&word, &tokenizer, &token, 11);
                break;

            // entrada e saida
            case INPUT:
                process.operation (&word, &tokenizer, &token, 12);
                break;
            case OUTPUT:
                process.operation (&word, &tokenizer, &token, 13);
                break;

            // fim do programa
            case STOP:
                output_code.push_back(14);  // insere codigo da operacao na linha de saida
                program_address++;          // incrementa endereco
                break;

            default: break;
        } not_directive = false;
    }
}

#endif
