// ----------------------------------------------------------------------------------------------------
//    PASSAGEM UNICA
// ----------------------------------------------------------------------------------------------------

#ifndef _H_SINGLEPASS_
#define _H_SINGLEPASS_

// tabelas de entrada para ligador
static Table definitions_table;
static Table usage_table;

// classe para processamento de operacoes
class Processor {
    public:

    // faz o processamento de operandos
    void operands (Analyze *word, istringstream *tokenizer, string *token) {
        if (*tokenizer >> *token) {     // insere na linha de saida a expressao seguinte a operacao
            output_code.push_back (word->check_operands (*token, program_address));
            program_address++;          // incrementa endereco
        }
    }
    // faz o processamento de uma operacao COPY
    void operation_COPY (Analyze *word, istringstream *tokenizer, string *token) {
        vector<string> argument;
        output_code.push_back(9);   // insere codigo da operacao na linha de saida
        program_address++;          // incrementa endereco

        if (*tokenizer >> *token)                           // pega a expressao seguinte ao codigo da operacao
            word->check_expression (*token, &argument);     // salva os argumentos
        for (unsigned int i=0; i < argument.size(); i++)    // para cada argumento
            operands (word, tokenizer, &argument[i]);       // processa os operandos
    }

    // faz o processamento de uma operacao
    void operation (Analyze *word, istringstream *tokenizer, string *token, int operation) {
        output_code.push_back (operation);  // insere codigo da operacao na linha de saida
        program_address++;                  // incrementa endereco
        operands (word, tokenizer, token);  // processa os operandos
    }
};

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
        } else {                                                    // se token for de outro tipo
            for (unsigned int i=0; i < label.size(); i++)
                symbol.insert_defined (label[i], program_address);  // declara rotulos salvos como definidos
                // nota: aqui ainda precisa atualizar linhas anteriores onde o rotulo aparece
            break;
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

            case D_SPACE:
                if ((tokenizer >> token) && word.check_number (token, false)) {
                    for (int i=0; i < word.number; i++)
                        output_code.push_back(0);
                    program_address += word.number;
                } else {
                    output_code.push_back(0);
                    program_address++;
                } break;

            case D_CONST:
                if ((tokenizer >> token) && word.check_number (token, true)) {
                    output_code.push_back (word.number);
                } else {
                    output_code.push_back(0);
                } program_address++;
                break;

            case D_PUBLIC:
                if (tokenizer >> token)
                    symbol.insert_public (token);
                break;

            default:
                not_directive = true;
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
                output_code.push_back(14);
                program_address++;
                break;

            default: break;
        }

        not_directive = false;
    }
}
#endif
