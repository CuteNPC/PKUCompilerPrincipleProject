#ifndef _IRBUILDER_HPP_
#define _IRBUILDER_HPP_

#include "ast.hpp"
#include "koopa.h"
#include "memory"
#include "symtab.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

class IRStmt;
class IRBlock;
class IRFunction;
class IRBuilder;

class IRStmt
{
  public:
    int hello;
};

class IRBlock
{
  public:
    vector<IRStmt *> irStmtVec;
};

class IRFunction
{
  public:
    string funcName;
    vector<IRBlock *> blockVec;
};

class IRGloData
{
  public:
    int hello;
};

class IRBuilder
{
  public:
    int currentTempCounter;

    IRBlock *currentBlock;

    string funcName;
    vector<IRBlock *> blockVec;

    vector<IRGloData *> dataVec;
    vector<IRFunction *> funcVec;

    IRBuilder();
    void buildFrom(CompUnitAST *ast, SymbolTable *symTab);
    void startFunc();
    void endFunc();
    std::string DumpToString();
};

#endif // !_IRBUILDER_HPP_
