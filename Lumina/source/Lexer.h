#ifndef LUMINA_LEXER_H
#define LUMINA_LEXER_H
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
using namespace std;
enum LexerStandart {
    End = -1,
    Macro = -2,
    function = -3,
    Class = -4,
    Name = -5,
    Number = -6,
    Operator = -7,
};
static string NameCopy; // Name bakar
static double NumberCopy;  // Number bakar
static int  Lexer() {
    static int LastChar = ' ';
    while (isspace(LastChar)){
        LastChar = getchar();
    }
    if (isdigit(LastChar) || LastChar == '.') {   // Number: [0-9.]+
        string NumStr;
        do {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.');
        NumberCopy = strtod(NumStr.c_str(), 0);
        return Number;
    }
    if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        NameCopy = LastChar;
        while (isalnum((LastChar = getchar()))){
            NameCopy += LastChar;
        }
        if (NameCopy == "macro")
            return Macro;
        if(NameCopy == "func")
            return function;
        if (NameCopy == "class")
            return Class;
        return Name;
    }
    /*
    if (ispunct(LastChar)) {
        char op = Operator;
        LastChar = getchar();
        return op;
    }
     */
    if (LastChar == EOF)
        return End;
    // Aksi takdirde karakteri ascii değeri olarak döndürmeniz yeterli.
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
};


#endif //LUMINA_LEXER_H
