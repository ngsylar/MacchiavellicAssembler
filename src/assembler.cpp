#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
using namespace std;

string line;
enum OPCODE {zero, ADD, SUB, MULT, DIV, JMP, JMPN, JMPP, JMPZ, COPY, LOAD, STORE, INPUT, OUTPUT, STOP};
static map<string, OPCODE> mapOP;

void stringSwitch () {
    mapOP["ADD"] = ADD;
    mapOP["SUB"] = SUB;
    mapOP["MULT"] = MULT;
    mapOP["DIV"] = DIV;
    mapOP["JMP"] = JMP;
    mapOP["JMPN"] = JMPN;
    mapOP["JMPP"] = JMPP;
    mapOP["JMPZ"] = JMPZ;
    mapOP["COPY"] = COPY;
    mapOP["LOAD"] = LOAD;
    mapOP["STORE"] = STORE;
    mapOP["INPUT"] = INPUT;
    mapOP["OUTPUT"] = OUTPUT;
    mapOP["STOP"] = STOP;
}

void decode () {
    istringstream tokenizer {line};
    string token;
    stringSwitch();

    while (tokenizer >> token) {
        // cout << token << endl;
        switch ( mapOP[token] ) {
            case ADD:       cout << token << endl;
                            break;
            case SUB:       cout << token << endl;
                            break;
            case MULT:      cout << token << endl;
                            break;
            case DIV:       cout << token << endl;
                            break;
            case JMP:       cout << token << endl;
                            break;
            case JMPN:      cout << token << endl;
                            break;
            case JMPP:      cout << token << endl;
                            break;
            case JMPZ:      cout << token << endl;
                            break;
            case COPY:      cout << token << endl;
                            break;
            case LOAD:      cout << token << endl;
                            break;
            case STORE:     cout << token << endl;
                            break;
            case INPUT:     cout << token << endl;
                            break;
            case OUTPUT:    cout << token << endl;
                            break;
            case STOP:      cout << token << endl;
                            break;
            default:        break;
        }
    }
}

int main () {
    ifstream file ("bin.asm");
    if ( file.is_open() ) {
        while ( !file.eof() ) {
            getline (file, line);
            decode();
            // cout << line << endl;
        }
        file.close();
    }
    else cout << "fail";
}
