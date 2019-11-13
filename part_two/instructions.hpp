// ----------------------------------------------------------------------------------------------------
//    DEFINICAO DE OPERACOES E DIRETIVAS RECONHECIDAS PELO MONTADOR
// ----------------------------------------------------------------------------------------------------

#ifndef _H_INSTRUCTIONS_
#define _H_INSTRUCTIONS_

// bibliotecas
#include <map>
using namespace std;

// definicao de opcodes e diretivas
enum E_OPCODE {ADD=1, SUB, MULT, DIV, JMP, JMPN, JMPP, JMPZ, COPY, LOAD, STORE, INPUT, OUTPUT, STOP};
enum E_DIRECTIVE {D_SECTION=1, D_SPACE, D_CONST, D_BEGIN, D_END, D_PUBLIC, D_EXTERN};
enum E_SECTION {S_TEXT=1, S_DATA};
static map<string, E_OPCODE> OPCODE;
static map<string, E_DIRECTIVE> DIRECTIVE;
static map<string, E_SECTION> SECTION;

// mapeia strings para serem usadas na estrutura condicional switch
void stringSwitch () {
    OPCODE["ADD"] = ADD;
    OPCODE["SUB"] = SUB;
    OPCODE["MULT"] = MULT;
    OPCODE["DIV"] = DIV;
    OPCODE["JMP"] = JMP;
    OPCODE["JMPN"] = JMPN;
    OPCODE["JMPP"] = JMPP;
    OPCODE["JMPZ"] = JMPZ;
    OPCODE["COPY"] = COPY;
    OPCODE["LOAD"] = LOAD;
    OPCODE["STORE"] = STORE;
    OPCODE["INPUT"] = INPUT;
    OPCODE["OUTPUT"] = OUTPUT;
    OPCODE["STOP"] = STOP;

    DIRECTIVE["SECTION"] = D_SECTION;
    DIRECTIVE["SPACE"] = D_SPACE;
    DIRECTIVE["CONST"] = D_CONST;
    DIRECTIVE["BEGIN"] = D_BEGIN;
    DIRECTIVE["END"] = D_END;
    DIRECTIVE["PUBLIC"] = D_PUBLIC;
    DIRECTIVE["EXTERN"] = D_EXTERN;
    
    SECTION["TEXT"] = S_TEXT;
    SECTION["DATA"] = S_DATA;
}

#endif
