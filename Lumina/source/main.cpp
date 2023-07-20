#include <iostream>
#include <unordered_map>
#include "Lexer.h"
using namespace std;
static int token;
static int getNextToken() {
    return token = Lexer();
}

class ASTmain{
public:
    virtual ~ASTmain() = default;
};
class ASTnumber : public ASTmain{
    double val;
public:
    ASTnumber(double val) : val(val) {}
};
/// VariableExprAST - Expression class for referencing a variable, like "a".
class ASTvariable : public ASTmain {
    string VariableName;
public:
    ASTvariable(const string &VariableName) : VariableName(VariableName) {}
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ASTmain {
    char Op;
    unique_ptr<ASTmain> LHS, RHS;
public:
    BinaryExprAST(char Op, unique_ptr<ASTmain> LHS, unique_ptr<ASTmain> RHS) : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};
/// CallExprAST - Expression class for function calls.
class CallExprAST : public ASTmain {
    std::string Callee;
    std::vector<std::unique_ptr<ASTmain>> Args;
public:
    CallExprAST(const string &Callee, vector<unique_ptr<ASTmain>> Args) : Callee(Callee), Args(std::move(Args)) {}
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST {
    string Name;
    vector<string> Args;
public:
    PrototypeAST(const string &Name, vector<string> Args) : Name(Name), Args(move(Args)) {} const string &getName() const { return Name; }
};
/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
    unique_ptr<PrototypeAST> Proto;
    unique_ptr<ASTmain> Body;

public:
    FunctionAST(unique_ptr<PrototypeAST> Proto, unique_ptr<ASTmain> Body) : Proto(move(Proto)), Body(move(Body)) {}
};


int main(){
    while (true){
        cout <<token<<endl;
        getNextToken();
    }
}