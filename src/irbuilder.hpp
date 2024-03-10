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
    IRStmt();
};

class IRBlock
{
  public:
    std::string blockName;
    bool deadBlock;
    std::vector<std::string> stmtVec;
    IRBlock();
    IRBlock(std::string blockName_, bool deadBlock_ = false,
            std::vector<std::string> stmtVec_ = std::vector<std::string>());
    void Dump(std::ostream &outStream = std::cout) const;
    friend std::ostream &operator<<(std::ostream &outStream, const IRBlock &block);
};

class IRFunction
{
  public:
    std::string funcName;
    std::string inputType;
    std::string outputType;
    std::vector<IRBlock *> blockVec;
    IRFunction();
    IRFunction(std::string funcName_, std::string inputType_, std::string outputType_,
               std::vector<IRBlock *> blockVec_ = std::vector<IRBlock *>());
    void Dump(std::ostream &outStream = std::cout) const;
    friend std::ostream &operator<<(std::ostream &outStream, const IRFunction &block);
};

class IRGloData
{
  public:
    int hello;
    /*TODO*/
};

class IRBuilder
{
  public:
    int currentTempCounter;

    IRBlock *currentBlock;

    std::string funcName;
    std::string inputType;
    std::string outputType;
    std::vector<IRBlock *> blockVec;

    std::vector<IRGloData *> dataVec;
    std::vector<IRFunction *> funcVec;

    IRBuilder();
    void buildFrom(CompUnitAST *ast, SymbolTable *symTab);
    void startFunc(std::string funcName_, std::string inputType_, std::string outputType_);
    void pushBlock(bool nextIsDeadBlock = false);
    void pushStmt(std::string stmt);
    void endFunc();
    std::string getNextIdent();
    void Dump(std::ostream &outStream) const;
    friend std::ostream &operator<<(std::ostream &outStream, const IRBuilder &block);
};

#endif // !_IRBUILDER_HPP_
