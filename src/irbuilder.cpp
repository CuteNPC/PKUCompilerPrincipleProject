#include "irbuilder.hpp"

const static char emptyMainKoopaIRString[] = "fun @%s(): i32 {\n%%entry:\n  ret %d\n}\n";

IRBuilder::IRBuilder() {}

IRBuilder::IRBuilder(CompUnitAST *ast_) { build(ast_); }

void IRBuilder::build(CompUnitAST *ast_)
{
    ast = ast_;
    funcName = ast->funcDef->funcName;
    returnValue = ast->funcDef->funcBody->itemVec[0]->stmt->lOrExp->primaryExp->constVal;
}

std::string IRBuilder::outputString()
{
    char buffer[0x100];
    sprintf(buffer, emptyMainKoopaIRString, funcName.c_str(), returnValue);
    outString = buffer;
    return outString;
}
