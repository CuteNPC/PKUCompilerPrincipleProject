#include "irbuilder.hpp"

const static char emptyMainKoopaIRString[] = "fun @%s(): i32 {\n%%entry:\n  ret %d\n}\n";

IRBuilder::IRBuilder() {}

IRBuilder::IRBuilder(const BaseAST &ast_)
{
    build(ast_);
}

void IRBuilder::build(const BaseAST &ast_)
{
    ast = &ast_;
    const CompUnitAST *cuastp = (const CompUnitAST *)ast;
    const FuncDefAST *fdastp = (const FuncDefAST *)(&*(cuastp->func_def));
    funcName = fdastp->ident;
    const BlockAST *bast = (const BlockAST *)(&*(fdastp->block));
    const StmtAST *sast = (const StmtAST *)(&*(bast->stmt));
    returnValue = sast->val;
}

std::string IRBuilder::outputString()
{
    char buffer[0x100];
    sprintf(buffer, emptyMainKoopaIRString, funcName.c_str(), returnValue);
    outString = buffer;
    return outString;
}
