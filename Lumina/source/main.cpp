#include <iostream>
#include <unordered_map>
#include "Lexer.h"
using namespace std;
static int token;
static int getNextToken() {
    return token = Lexer();
}
namespace {
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
}
//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//
static int CurTok;
static int gnt() {
    return CurTok = Lexer();
}
static map<char , int> operatorPrecedence;
// önceliği alır
static int tokPreden(){
    if(!isascii(CurTok)){
        return -1;
    }
    int TokPrec = operatorPrecedence[CurTok];
    if(TokPrec <= 0){
        return -1;
    }
    return TokPrec;
}
/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ASTmain> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}
static std::unique_ptr<ASTmain> ParseExpression();
/// numberexpr ::= number
static std::unique_ptr<ASTmain> Parsernumber() {
    auto Result = std::make_unique<ASTnumber>(NumberCopy);
    gnt(); // consume the number
    return std::move(Result);
}
/// parenexpr ::= '(' expression ')'
static unique_ptr<ASTmain> ParseParn(){
    gnt(); // eat (.
    auto V = ParseExpression();
    if (!V)
        return nullptr;
    if (CurTok != ')')
        return LogError("expected ')'");
    gnt(); // eat ).
    return V;
}
static std::unique_ptr<ASTmain> ParseName() {
    std::string IdName = NameCopy;
    gnt(); // eat identifier.
    if (CurTok != '(') // Simple variable ref.
        return std::make_unique<ASTvariable>(IdName);
    // Call.
    gnt(); // eat (
    std::vector<std::unique_ptr<ASTmain>> Args;
    if (CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;
            if (CurTok == ')')
                break;
            if (CurTok != ',')
                return LogError("Expected ')' or ',' in argument list");
            gnt();
        }
    }
    // Eat the ')'.
    gnt();
    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<ASTmain> ParsePrimary() {
    switch (CurTok) {
        case Name:
            return ParseName();
        case Number:
            cout << "sayi geldi";
            return Parsernumber();
        case '(':
            return ParseParn();
        default:
            return LogError("unknown token when expecting an expression");
    }
}
/// binoprhs
///   ::= ('+' primary)*
static unique_ptr<ASTmain> ParseBinOpRHS(int ExprPrec,unique_ptr<ASTmain> LHS) {
    // If this is a binop, find its precedence.
    while (true) {
        int TokPrec = tokPreden();
        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec)
            return LHS;
        // Okay, we know this is a binop.
        int BinOp = CurTok;
        gnt(); // eat binop
        // Parse the primary expression after the binary operator.
        cout << TokPrec;
        auto RHS = ParsePrimary();
        if (!RHS)
            return nullptr;
        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = tokPreden();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }
        // Merge LHS/RHS.
        LHS =
                std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}
static std::unique_ptr<ASTmain> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;
    return ParseBinOpRHS(0, std::move(LHS));
}
/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
    if (CurTok != Name) {
        return LogErrorP("Expected function name in prototype");
    }
    string FnName = NameCopy;
    getNextToken();
    if (CurTok != '(') {
        return LogErrorP("Expected '(' in prototype");
    }
    vector<std::string> ArgNames;
    while (gnt() == Name)
        ArgNames.push_back(NameCopy);
    if (CurTok != ')')
        return LogErrorP("Expected ')' in prototype");
    // success.
    gnt(); // eat ')'.
    return make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}
/// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
    gnt(); // eat def.
    auto Proto = ParsePrototype();
    if (!Proto)
        return nullptr;
    if (auto E = ParseExpression())
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    return nullptr;
}
/// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        // Make an anonymous proto.
        auto Proto = std::make_unique<PrototypeAST>("__anon_expr",
                                                    std::vector<std::string>());
        return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
    }
    return nullptr;
}
/// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
    gnt(); // eat extern.
    return ParsePrototype();
}
//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//
static void HandleDefinition() {
    if (ParseDefinition()) {
        cout << "fonksiyona giriyor" << endl;
        fprintf(stderr, "Parsed a function definition.\n");
    } else {
        // Skip token for error recovery.
        gnt();
    }
}
static void HandleExtern() {
    if (ParseExtern()) {
        cout << "name giriyor";
        fprintf(stderr, "Parsed an name\n");
    } else {
        // Skip token for error recovery.
        gnt();
    }
}
static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (ParseTopLevelExpr()) {
        fprintf(stderr, "Parsed a top-level expr\n");
    } else {
        // Skip token for error recovery.
        gnt();
    }
}
/// top ::= definition | external | expression | ';'
static void MainLoop() {
    while (true) {
        fprintf(stderr, "ready> ");
        switch (CurTok) {
            case End:
                return;
            case ';': // ignore top-level semicolons.
                gnt();
                break;
            case function:
                HandleDefinition();
                break;
            case Name:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}
// ===---------------------------------------------------------------------===//
int main() {
    // Install standard binary operators.
    // 1 is lowest precedence.
    operatorPrecedence['<'] = 10;
    operatorPrecedence['+'] = 20;
    operatorPrecedence['-'] = 20;
    operatorPrecedence['*'] = 40; // highest.
    // Prime the first token.
    fprintf(stderr, "Lumina> ");
    gnt();
    // Run the main "interpreter loop" now.
    MainLoop();
    return 0;
}