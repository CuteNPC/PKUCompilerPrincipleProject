#include "irbuilder.hpp"

using namespace std;

const static char emptyMainKoopaIRString[] = "fun @%s(): i32 {\n%%entry:\n  ret %d\n}\n";

/* IRBuilder */

IRBuilder::IRBuilder()
    : currentTempCounter(0), currentBlock(NULL), funcName(), blockVec(), dataVec(), funcVec()
{
}

void IRBuilder::buildFrom(CompUnitAST *ast, SymbolTable *symTab)
{
    symTab->resetCursor();
    ast->buildIR(this, symTab);
    symTab->resetCursor();
}

string IRBuilder::DumpToString() { return string(emptyMainKoopaIRString); }

void IRBuilder::startFunc()
{
    currentTempCounter = 0;
    currentBlock = new IRBlock();
    funcName.clear();
    blockVec.clear();
}

void IRBuilder::endFunc()
{
    currentTempCounter = 0;
    blockVec.push_back(currentBlock);
    currentBlock = NULL;
    IRFunction *irFunction = new IRFunction();
    /*TODO 构建irFunction*/
    blockVec.clear();
    funcVec.push_back(irFunction);
}

/* END */
