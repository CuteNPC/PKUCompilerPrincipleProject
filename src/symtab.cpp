#include "symtab.hpp"
#include "ast.hpp"

SymbolEntry::SymbolEntry()
    : symTab(NULL), type(TypeEnum::TYPE_INT), defi(DefiEnum::DEFI_NONE), ident(), funcName(),
      blockVecIndex(), blockLineIndex(0), arrayDimVec(), initvalArray(NULL), funcPara(false)
{
}

SymbolEntry::SymbolEntry(SymbolTable *symTab_, TypeEnum type_, DefiEnum defi_, string funcName_,
                         vector<int> blockVecIndex_, int blockLineIndex_,
                         DataLValIdentAST *defIdent_, DataInitvalAST *initval_, bool funcPara_)
    : symTab(symTab_), type(type_), defi(defi_), funcName(funcName_), blockVecIndex(blockVecIndex_),
      blockLineIndex(blockLineIndex_), initval(0), initvalArray(NULL), funcPara(funcPara_)
{
    ident = defIdent_->ident;
    int arrayDimVecCum = 1;
    if (defIdent_->emptyValStart)
        arrayDimVec.push_back(-1);
    for (ExpAST *&exp_ : defIdent_->expVec)
    {
        int expval = exp_->forceCalc(symTab);
        arrayDimVec.push_back(expval);
        arrayDimVecCum *= expval;
    }
    if (initval_)
    {
        if (defIdent_->expVec.size() == 0)
        {
            initval = initval_->exp->forceCalc(symTab);
            initvalArray = NULL;
        }
        else
        {
            initval = 0;
            initvalArray = new int[arrayDimVecCum];
            // for (DataInitvalAST *init : initval_->initVec)
            // {
            // init = init;
            // /*TODO 设置初始化信息*/;
            // }
        }
    }
}

SymbolEntry::~SymbolEntry() {}

bool SymbolEntry::isArray() const { return !arrayDimVec.empty(); }

bool SymbolEntry::isGlobal() const { return blockVecIndex.empty(); }

bool SymbolEntry::isFuncPara() const { return funcPara; }

bool SymbolEntry::isEmptyStartArray() const
{
    if (arrayDimVec.empty())
        return false;
    return (arrayDimVec.front() == -1);
}

SymbolTable::SymbolTable()
    : currentFuncName(), currentBlockVecIndex(), currentBlockVecIndexTail(0),
      currentBlockLineIndex(0), symVec()
{
}

void SymbolTable::append(SymbolEntry *sym_) { symVec.push_back(sym_); }

void SymbolTable::enterBlock()
{
    currentBlockVecIndex.push_back(currentBlockVecIndexTail);
    currentBlockVecIndexTail = 0;
}

void SymbolTable::leaveBlock()
{
    currentBlockVecIndexTail = currentBlockVecIndex.back() + 1;
    currentBlockVecIndex.pop_back();
}

void SymbolTable::antiLeaveBlock()
{
    currentBlockVecIndexTail = currentBlockVecIndex.back();
    currentBlockVecIndex.pop_back();
}

vector<SymbolEntry *> &SymbolTable::Vec() { return symVec; }

SymbolEntry *SymbolTable::match(string ident_, TypeEnum type_, DefiEnum defi_, string funcName_,
                                const vector<int> &blockVecIndex_) const
{
    for (auto iter = symVec.rbegin(); iter != symVec.rend(); iter++)
    {
        SymbolEntry *sym = *iter;
        if (sym->type != type_)
            continue;
        // if (sym->defi != TypeEnum::TYPE_CONST)
        // continue;
        if (sym->ident != ident_)
            continue;
        if (sym->funcName != funcName_)
            continue;
        int entryBlockVecIndexSize = sym->blockVecIndex.size();
        if (entryBlockVecIndexSize > blockVecIndex_.size())
            continue;
        for (int i = 0; i < entryBlockVecIndexSize; i++)
            if (sym->blockVecIndex[i] != blockVecIndex_[i])
                continue;
        return sym;
    }
    return NULL;
}

SymbolTable::~SymbolTable()
{
    for (SymbolEntry *sym : symVec)
        delete sym;
}
