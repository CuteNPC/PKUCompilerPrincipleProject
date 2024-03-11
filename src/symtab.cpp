#include "symtab.hpp"
#include "ast.hpp"
#include <assert.h>

/* SymbolEntry */

SymbolEntry::SymbolEntry()
    : symTab(NULL), type(TypeEnum::TYPE_INT), defi(DefiEnum::DEFI_NONE), ident(), funcName(),
      blockVecIndex(), blockLineIndex(0), arrayDimVec(), initvalArray(), funcPara(false)
{
}

SymbolEntry::SymbolEntry(SymbolTable *symTab_, TypeEnum type_, DefiEnum defi_,
                         std::string funcName_, std::vector<int> blockVecIndex_,
                         int blockLineIndex_, std::string ident_, std::vector<int> arrayDimVec_,
                         int initval_, std::vector<int> initvalArray_, bool funcPara_)
    : symTab(symTab_), type(type_), defi(defi_), ident(ident_), funcName(funcName_),
      blockVecIndex(blockVecIndex_), blockLineIndex(blockLineIndex_), arrayDimVec(arrayDimVec_),
      initval(initval_), initvalArray(initvalArray_), funcPara(funcPara_)
{
}

SymbolEntry::~SymbolEntry() {}

bool SymbolEntry::isArray() const { return !arrayDimVec.empty(); }

bool SymbolEntry::isGlobal() const { return blockVecIndex.empty(); }

bool SymbolEntry::isConst() const { return defi == DefiEnum::DEFI_CONST; }

bool SymbolEntry::isFuncPara() const { return funcPara; }

bool SymbolEntry::isEmptyStartArray() const
{
    if (arrayDimVec.empty())
        return false;
    return (arrayDimVec.front() == -1);
}

void SymbolEntry::Dump(std::ostream &outStream) const
{
    outStream << "    "
              << "Defi : " << defiName[defi] << std::endl;
    outStream << "    "
              << "Type : " << typeName[type] << std::endl;
    outStream << "    "
              << "Ident: " << ident << std::endl;
    outStream << "    "
              << "Func : " << funcName << std::endl;
    outStream << "    "
              << "funcPara: " << (funcPara ? "True" : "False") << std::endl;
    outStream << "    "
              << "initVal : " << initval << std::endl;

    outStream << "    "
              << "blockVecIndex: [";
    for (int e : blockVecIndex)
        outStream << e << ", ";
    outStream << ']' << std::endl;

    outStream << "    "
              << "arrayDimVec  : [";
    for (int e : arrayDimVec)
        outStream << e << ", ";
    outStream << ']' << std::endl;
}

std::string SymbolEntry::getIRVarName(bool isParamEnd) const
{
    auto replaceFunc = [](std::string originalString, std::string searchString,
                          std::string replaceString) -> std::string
    {
        size_t pos = 0;
        while ((pos = originalString.find(searchString, pos)) != std::string::npos)
        {
            originalString.replace(pos, searchString.length(), replaceString);
            pos += replaceString.length();
        }
        return originalString;
    };

    std::string newVarName("@");
    newVarName += replaceFunc(ident, std::string("_"), std::string("__"));
    for (int number : blockVecIndex)
        newVarName += std::string("_") + std::to_string(number);
    if (isParamEnd)
    {
        assert(funcPara);
        newVarName += std::string("_") + std::string("isparam");
    }
    return newVarName;
}

std::ostream &operator<<(std::ostream &outStream, const SymbolEntry &entry)
{
    entry.Dump(outStream);
    return outStream;
}

/* SymbolTable */

SymbolTable::SymbolTable()
    : currentFuncName(), currentBlockVecIndex(), currentBlockVecIndexTail(0),
      currentBlockLineIndex(0), symVec(), funcTypeIsVoid()
{
}

SymbolTable::~SymbolTable()
{
    for (SymbolEntry *sym : symVec)
        delete sym;
}

void SymbolTable::resetCursor()
{
    currentFuncName.clear();
    currentBlockVecIndex.clear();
    currentBlockVecIndexTail = 0;
    currentBlockLineIndex = 0;
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

std::vector<SymbolEntry *> &SymbolTable::Vec() { return symVec; }

SymbolEntry *SymbolTable::match(std::string ident_, TypeEnum type_, DefiEnum defi_) const
{
    return match(ident_, type_, defi_, currentFuncName, currentBlockVecIndex);
}

SymbolEntry *SymbolTable::match(std::string ident_, TypeEnum type_, DefiEnum defi_,
                                std::string funcName_, const std::vector<int> &blockVecIndex_) const
{
    for (auto iter = symVec.rbegin(); iter != symVec.rend(); iter++)
    {
        SymbolEntry *sym = *iter;
        if (sym->type != type_)
            continue;
        // if (sym->defi != defi_)
        // continue;
        if (sym->ident != ident_)
            continue;
        // if (sym->funcName != funcName_)
        // continue;
        int entryBlockVecIndexSize = sym->blockVecIndex.size();
        if (entryBlockVecIndexSize > blockVecIndex_.size())
            continue;
        bool ok = true;
        for (int i = 0; i < entryBlockVecIndexSize; i++)
            if (sym->blockVecIndex[i] != blockVecIndex_[i])
                ok = false;
        if (ok)
            return sym;
    }
    assert(false);
    return NULL;
}

void SymbolTable::buildFrom(CompUnitAST *ast)
{
    resetCursor();
    for (auto p = libFuncDecl; (*p)[0]; p++)
        funcTypeIsVoid.insert(
            std::make_pair(std::string((*p)[0]), (std::string((*p)[2]).length() == 0)));
    ast->setSymbolTable(this);
    resetCursor();
}

void SymbolTable::Dump(std::ostream &outStream) const
{
    outStream << "SymbolTable: " << std::endl;
    int cnt = 0;
    for (SymbolEntry *elem : symVec)
    {
        outStream << cnt << std::endl;
        elem->Dump(outStream);
        cnt++;
    }
}

bool SymbolTable::funcRetVoid(std::string funcName_)
{
    if (funcTypeIsVoid.count(funcName_))
        return funcTypeIsVoid.at(funcName_);
    else
        assert(false);
    return false;
}

std::ostream &operator<<(std::ostream &outStream, const SymbolTable &symTab)
{
    symTab.Dump(outStream);
    return outStream;
}

/* END */
