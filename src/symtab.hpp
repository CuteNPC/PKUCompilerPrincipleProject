#ifndef _SYMTAB_HPP_
#define _SYMTAB_HPP_

#include "keyword.hpp"
#include <iostream>
#include <map>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

class SymbolEntry;
class SymbolTable;
class DataLValIdentAST;
class DataInitvalAST;
class CompUnitAST;
class FuncDefAST;

class SymbolEntry
{
  public:
    SymbolTable *symTab;
    TypeEnum type;
    DefiEnum defi;
    std::string ident;
    std::string funcName;
    std::vector<int> blockVecIndex;
    int blockLineIndex;
    std::vector<int> arrayDimVec;
    int initval;
    std::vector<int> initvalArray;
    bool funcPara;

    SymbolEntry();
    SymbolEntry(SymbolTable *symTab_, TypeEnum type_, DefiEnum defi_, std::string funcName_,
                std::vector<int> blockVecIndex_, int blockLineIndex_, std::string ident_,
                std::vector<int> arrayDimVec_, int initval_ = 0,
                std::vector<int> initvalArray_ = std::vector<int>(), bool funcPara_ = false);
    ~SymbolEntry();
    bool isArray() const;
    bool isGlobal() const;
    bool isFuncPara() const;
    bool isEmptyStartArray() const;
    void Dump(std::ostream &outStream = std::cout) const;
    std::string getIRVarName(bool isParamEnd = false) const;
    friend std::ostream &operator<<(std::ostream &outStream, const SymbolEntry &ast);
};

class SymbolTable
{
  public:
    std::string currentFuncName;
    std::vector<int> currentBlockVecIndex;
    int currentBlockVecIndexTail;
    int currentBlockLineIndex; /* no used */
    std::vector<SymbolEntry *> symVec;
    std::unordered_map<std::string, bool> funcTypeIsVoid;

    SymbolTable();
    ~SymbolTable();
    SymbolEntry *match(std::string ident, TypeEnum type_, DefiEnum defi_) const;
    SymbolEntry *match(std::string ident, TypeEnum type_, DefiEnum defi_, std::string funcName_,
                       const std::vector<int> &blockVecIndex_) const;
    std::vector<SymbolEntry *> match() const;
    std::vector<SymbolEntry *> match(std::string funcName_,
                                     const std::vector<int> &blockVecIndex_) const;
    void append(SymbolEntry *sym_);
    void enterBlock();
    void leaveBlock();
    void antiLeaveBlock();
    std::vector<SymbolEntry *> &Vec();
    void buildFrom(CompUnitAST *ast);
    void resetCursor();
    bool funcRetVoid(std::string funcName);
    void Dump(std::ostream &outStream = std::cout) const;
    friend std::ostream &operator<<(std::ostream &outStream, const SymbolTable &ast);
};

#endif // !_SYMTAB_HPP_
