#include "symtab.hpp"
#include "ast.hpp"

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

std::string SymbolEntry::getIRVarName() const
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
    if (funcPara)
        newVarName += std::string("_") + std::string("isparam");
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
      currentBlockLineIndex(0), symVec()
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

std::vector<SymbolEntry *> SymbolTable::match() const
{
    return match(currentFuncName, currentBlockVecIndex);
}

std::vector<SymbolEntry *> SymbolTable::match(std::string funcName_,
                                              const std::vector<int> &blockVecIndex_) const
{
    std::vector<SymbolEntry *> res;
    int len = blockVecIndex_.size();
    for (SymbolEntry *sym : symVec)
    {
        if (sym->funcName != funcName_)
            continue;
        if (len != sym->blockVecIndex.size())
            continue;
        for (int i = 0; i < len; i++)
            if (blockVecIndex_[i] != sym->blockVecIndex[i])
                continue;
        res.push_back(sym);
    }
    return res;
}

void SymbolTable::buildFrom(CompUnitAST *ast)
{
    resetCursor();
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

std::ostream &operator<<(std::ostream &outStream, const SymbolTable &symTab)
{
    symTab.Dump(outStream);
    return outStream;
}

/* END */
