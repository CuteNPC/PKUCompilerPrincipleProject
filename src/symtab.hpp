#ifndef _SYMTAB_HPP_
#define _SYMTAB_HPP_

#include "keyword.hpp"
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
    int* initvalArray;
    bool funcPara;

    SymbolEntry();
    SymbolEntry(SymbolTable *symTab_, TypeEnum type_, DefiEnum defi_, string funcName_,
                vector<int> blockVecIndex_, int blockLineIndex_, DataLValIdentAST *defIdent_,
                DataInitvalAST *initval_ = NULL, bool funcPara_ = false);
    ~SymbolEntry();
    bool isArray() const;
    bool isGlobal() const;
    bool isFuncPara() const;
    bool isEmptyStartArray() const;
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
    SymbolEntry *match(string ident, TypeEnum type_, DefiEnum defi_, string funcName_,
                       const vector<int> &blockVecIndex_) const;
    void append(SymbolEntry *sym_);
    void enterBlock();
    void leaveBlock();
    void antiLeaveBlock();
    vector<SymbolEntry *> &Vec();
    ~SymbolTable();
};

#endif // !_SYMTAB_HPP_
