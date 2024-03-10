#ifndef _IRBUILDER_HPP_
#define _IRBUILDER_HPP_

#include "ast.hpp"
#include "koopa.h"
#include "memory"
#include "symtab.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

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
    std::vector<IRStmt *> irStmtVec;
};

class IRFunction
{
  public:
    std::string funcName;
    std::vector<IRBlock *> blockVec;
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

    std::string funcName;
    std::vector<IRBlock *> blockVec;

    std::vector<IRGloData *> dataVec;
    std::vector<IRFunction *> funcVec;

    IRBuilder();
    void buildFrom(CompUnitAST *ast, SymbolTable *symTab);
    void startFunc();
    void endFunc();
    std::string DumpToString();
};

#endif // !_IRBUILDER_HPP_
