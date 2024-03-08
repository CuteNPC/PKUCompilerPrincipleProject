#ifndef _SYMTAB_HPP_
#define _SYMTAB_HPP_

#include "keyword.hpp"
#include <iostream>
#include <map>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class SymbolEntry;
class SymbolTable;
class DataLValIdentAST;
class DataInitvalAST;

class SymbolEntry
{
  public:
    SymbolTable *symTab;
    TypeEnum type;
    DefiEnum defi;
    string ident;
    string funcName;
    vector<int> blockVecIndex;
    int blockLineIndex;
    vector<int> arrayDimVec;
    int initval;
    vector<int> initvalArray;
    bool funcPara;

    SymbolEntry();
    SymbolEntry(SymbolTable *symTab_, TypeEnum type_, DefiEnum defi_, std::string funcName_,
                vector<int> blockVecIndex_, int blockLineIndex_, std::string ident_,
                std::vector<int> arrayDimVec_, int initval_ = 0,
                std::vector<int> initvalArray_ = std::vector<int>(), bool funcPara_ = false);
    ~SymbolEntry();
    bool isArray() const;
    bool isGlobal() const;
    bool isFuncPara() const;
    bool isEmptyStartArray() const;
    void Dump(std::ostream &outStream = std::cout) const;
    friend std::ostream &operator<<(std::ostream &outStream, const SymbolEntry &ast);
};

class SymbolTable
{
  public:
    string currentFuncName;
    vector<int> currentBlockVecIndex;
    int currentBlockVecIndexTail;
    int currentBlockLineIndex;
    vector<SymbolEntry *> symVec;

    SymbolTable();
    ~SymbolTable();
    SymbolEntry *match(string ident, TypeEnum type_, DefiEnum defi_, string funcName_,
                       const vector<int> &blockVecIndex_) const;
    void append(SymbolEntry *sym_);
    void enterBlock();
    void leaveBlock();
    void antiLeaveBlock();
    vector<SymbolEntry *> &Vec();
    void Dump(std::ostream &outStream = std::cout) const;
    friend std::ostream &operator<<(std::ostream &outStream, const SymbolTable &ast);
};

#endif // !_SYMTAB_HPP_
