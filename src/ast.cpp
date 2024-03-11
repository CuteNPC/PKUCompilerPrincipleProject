#include "ast.hpp"
#include "define.hpp"
#include "irbuilder.hpp"
#include "symtab.hpp"
#include <assert.h>
#include <iomanip>

/* Indent */

class Indent
{
    uint32_t val;

  public:
    Indent(uint32_t val_ = 0);
    friend std::ostream &operator<<(std::ostream &outStream, const Indent &ast);
};

Indent::Indent(uint32_t val_) : val(val_){};

std::ostream &operator<<(std::ostream &outStream, const Indent &indent)
{
    for (int i = 0; i < indent.val; i++)
        outStream << DUMP_INDENT_STRING;
    return outStream;
}

/* BaseAST */

void BaseAST::Dump(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent) << getClassName() << std::endl;
    outStream << Indent(indent) << "{" << std::endl;
    DumpContent(outStream, indent);
    outStream << Indent(indent) << "}" << std::endl;
}

std::ostream &operator<<(std::ostream &outStream, const BaseAST &ast)
{
    ast.Dump(outStream, 0);
    return outStream;
}

/* CompUnitAST */

CompUnitAST::CompUnitAST() : declVec(), funcVec(){};

CompUnitAST::CompUnitAST(DataDeclAST *decl_) : declVec(), funcVec()
{
    if (decl_)
        append(decl_);
}

CompUnitAST::CompUnitAST(FuncDefAST *func_) : declVec(), funcVec()
{
    if (func_)
        append(func_);
};

CompUnitAST::~CompUnitAST()
{
    for (DataDeclAST *decl : declVec)
        delete decl;
    for (FuncDefAST *func : funcVec)
        delete func;
};

void CompUnitAST::append(DataDeclAST *decl_) { declVec.push_back(decl_); }

void CompUnitAST::append(FuncDefAST *func_) { funcVec.push_back(func_); }

const char *CompUnitAST::getClassName() const { return "CompUnitAST"; }

void CompUnitAST::DumpContent(std::ostream &outStream, int indent) const
{
    for (DataDeclAST *decl : declVec)
        decl->Dump(outStream, indent + 1);
    for (FuncDefAST *func : funcVec)
        func->Dump(outStream, indent + 1);
}

void CompUnitAST::setSymbolTable(SymbolTable *symTab)
{
    for (DataDeclAST *decl : declVec)
        decl->setSymbolTable(symTab);
    for (DataDeclAST *decl : declVec)
        delete decl;
    declVec.clear();
    for (FuncDefAST *func : funcVec)
        symTab->funcTypeIsVoid.insert(
            std::make_pair(func->funcName, func->funcType == TypeEnum::TYPE_VOID));

    for (FuncDefAST *func : funcVec)
        func->setSymbolTable(symTab);
}

void CompUnitAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab)
{
    /*
    任务
    1.找出所有的 全局变量整数，global alloc
    2.找出所有的 非（局部变量）的数组，global alloc
    */
    for (SymbolEntry *sym : symTab->symVec)
    {
        if (sym->isGlobal() && !sym->isConst() && !sym->isArray())
        {
            std::string newName = sym->getIRVarName();
            std::string typeName = std::string("i32");
            std::string initVal = std::to_string(sym->initval);
            std::string stmt =
                std::string("global ") + newName + " = alloc " + typeName + ", " + initVal;
            irBuilder->dataVec.push_back(stmt);
        }

        if ((sym->isGlobal() || sym->isConst()) && sym->isArray())
        {
            std::string newName = sym->getIRVarName();
            std::string typeName = irBuilder->getIRType(sym->arrayDimVec);
            std::string initVal =
                irBuilder->aggregate1DtoNDString(sym->initvalArray, sym->arrayDimVec);
            std::string stmt =
                std::string("global ") + newName + " = alloc " + typeName + ", " + initVal;
            irBuilder->dataVec.push_back(stmt);
        }
    }

    for (FuncDefAST *func : funcVec)
        func->buildIR(irBuilder, symTab);
}

/* FuncDefAST */

FuncDefAST::FuncDefAST() : funcType(TypeEnum::TYPE_NONE), funcName(), funcBody(NULL){};

FuncDefAST::FuncDefAST(TypeEnum funcType_, std::string funcName_, FuncFParamsAST *paras_,
                       BlockAST *funcBody_)
    : funcType(funcType_), funcName(funcName_), paras(paras_), funcBody(funcBody_){};

FuncDefAST::FuncDefAST(TypeEnum funcType_, const char *funcName_, FuncFParamsAST *paras_,
                       BlockAST *funcBody_)
    : funcType(funcType_), funcName(funcName_), paras(paras_), funcBody(funcBody_){};

FuncDefAST::~FuncDefAST()
{
    if (funcBody)
        delete funcBody;
}

const char *FuncDefAST::getClassName() const { return "FuncDefAST"; }

void FuncDefAST::DumpContent(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent + 1) << "funcType: " << typeName[(int)(funcType)] << std::endl;
    outStream << Indent(indent + 1) << "funcName: " << funcName << std::endl;
    paras->Dump(outStream, indent + 1);
    funcBody->Dump(outStream, indent + 1);
}

void FuncDefAST::setSymbolTable(SymbolTable *symTab)
{
    symTab->currentFuncName = funcName;
    paras->setSymbolTable(symTab);
    funcBody->setSymbolTable(symTab);
    symTab->currentFuncName.clear();
}

void FuncDefAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab)
{
    symTab->currentFuncName = funcName;

    std::string inputType = paras->buildIRRetString(irBuilder, symTab);
    std::string outputType = std::string(funcType == TypeEnum::TYPE_INT ? ": i32" : "");

    irBuilder->startFunc(funcName, inputType, outputType);
    funcBody->buildIR(irBuilder, symTab);
    if (funcType == TypeEnum::TYPE_VOID)
    {
        std::string stmt = std::string("ret");
        irBuilder->pushStmt(stmt);
        irBuilder->pushAndGetBlock(true);
    }
    irBuilder->endFunc();

    symTab->currentFuncName.clear();
}

/* BlockAST */

BlockAST::BlockAST() : itemVec(), vecIndex(), lineIndex() {}

void BlockAST::append(BlockItemAST *item_)
{
    if (item_)
        itemVec.push_back(item_);
}

void BlockAST::setIndex(std::vector<int> vecIndex_, int lineIndex_)
{
    vecIndex = vecIndex_;
    lineIndex = lineIndex_;
}

BlockAST::~BlockAST()
{
    for (BlockItemAST *elem : itemVec)
        delete elem;
}

const char *BlockAST::getClassName() const { return "BlockAST"; }

void BlockAST::DumpContent(std::ostream &outStream, int indent) const
{
    for (BlockItemAST *elem : itemVec)
        elem->Dump(outStream, indent + 1);
    for (StmtAST *elem : stmtVec)
        elem->Dump(outStream, indent + 1);
}

void BlockAST::setSymbolTable(SymbolTable *symTab)
{
    symTab->enterBlock();
    vecIndex = symTab->currentBlockVecIndex;
    for (BlockItemAST *item : itemVec)
    {
        item->setSymbolTable(symTab);
        if (item->itemEnum == BlockItemEnum::BLOCK_DECL)
        {
            for (DataDefAST *def : item->decl->defVec)
            {
                if (def->stmtAfterSym != NULL)
                {
                    def->stmtAfterSym->setSymbolTable(symTab);
                    stmtVec.push_back(def->stmtAfterSym);
                    def->stmtAfterSym = NULL;
                }
            }
        }
        if (item->itemEnum == BlockItemEnum::BLOCK_STMT)
        {
            item->stmt->setSymbolTable(symTab);
            stmtVec.push_back(item->stmt);
            item->stmt = NULL;
        }
        delete item;
    }
    itemVec.clear();
    symTab->leaveBlock();
}

void BlockAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab)
{
    symTab->enterBlock();

    if (symTab->currentBlockVecIndex.size() == 1)
    {
        /*函数参数分配空间与赋值，要求：当前在函数块的开头，是属于这个函数块的，是参数*/
        for (SymbolEntry *sym : symTab->symVec)
        {
            if (sym->isFuncPara() && sym->blockVecIndex[0] == symTab->currentBlockVecIndex[0])
            {
                std::string newName = sym->getIRVarName();
                std::string typeName = irBuilder->getIRType(sym->arrayDimVec);
                std::string stmt = newName + std::string(" = alloc ") + typeName;
                irBuilder->pushStmt(stmt);
                std::string paraName = sym->getIRVarName(true);
                stmt = std::string("store ") + paraName + ", " + newName;
                irBuilder->pushStmt(stmt);
            }
        }

        /*局部变量数组，非函数参数的，并且块相同*/
        for (SymbolEntry *sym : symTab->symVec)
        {
            if (!sym->isFuncPara() && sym->isArray() && !sym->isGlobal() && !sym->isConst() &&
                sym->blockVecIndex[0] == symTab->currentBlockVecIndex[0])
            {
                std::string newName = sym->getIRVarName();
                std::string typeName = irBuilder->getIRType(sym->arrayDimVec);
                std::string stmt = newName + std::string(" = alloc ") + typeName;
                irBuilder->pushStmt(stmt);
            }
        }
    }

    for (SymbolEntry *sym : symTab->symVec)
    {
        /*变量赋值*/
        if (!sym->isArray() && !sym->isFuncPara() && !sym->isGlobal() && !sym->isConst())
        {
            int len = symTab->currentBlockVecIndex.size();
            if (len != sym->blockVecIndex.size())
                continue;
            for (int i = 0; i < len; i++)
                if (symTab->currentBlockVecIndex[i] != sym->blockVecIndex[i])
                    continue;
            std::string newName = sym->getIRVarName();
            std::string stmt = newName + std::string(" = alloc i32");
            irBuilder->pushStmt(stmt);
        }
    }

    for (StmtAST *stmt : stmtVec)
        stmt->buildIR(irBuilder, symTab);

    symTab->leaveBlock();
}

/* BlockItemAST */

BlockItemAST::BlockItemAST() : itemEnum(BlockItemEnum::BLOCK_NONE), ptr(NULL) {}

BlockItemAST::BlockItemAST(DataDeclAST *decl_) : itemEnum(BlockItemEnum::BLOCK_DECL), decl(decl_) {}

BlockItemAST::BlockItemAST(StmtAST *stmt_) : itemEnum(BlockItemEnum::BLOCK_STMT), stmt(stmt_) {}

BlockItemAST::~BlockItemAST()
{
    if (itemEnum == BlockItemEnum::BLOCK_DECL)
        delete decl;
    if (itemEnum == BlockItemEnum::BLOCK_STMT)
        delete stmt;
}

const char *BlockItemAST::getClassName() const { return "BlockItemAST"; }

void BlockItemAST::DumpContent(std::ostream &outStream, int indent) const
{
    if (itemEnum == BlockItemEnum::BLOCK_NONE)
        outStream << Indent(indent + 1) << "NONE" << std::endl;
    if (itemEnum == BlockItemEnum::BLOCK_DECL)
        decl->Dump(outStream, indent + 1);
    if (itemEnum == BlockItemEnum::BLOCK_STMT)
        stmt->Dump(outStream, indent + 1);
}

void BlockItemAST::setSymbolTable(SymbolTable *symTab)
{
    if (itemEnum == BlockItemEnum::BLOCK_DECL)
        decl->setSymbolTable(symTab);
    // if (itemEnum == BlockItemEnum::BLOCK_STMT)
    // stmt->setSymbolTable(symTab);
}

void BlockItemAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab) {}

/* StmtAST */

StmtAST::StmtAST()
    : st(StmtEnum::STMT_NONE), lVal(NULL), ptr(NULL), mainStmt(NULL), elseStmt(NULL), initvalArray()
{
}

StmtAST::StmtAST(StmtEnum st_)
    : st(st_), lVal(NULL), ptr(NULL), mainStmt(NULL), elseStmt(NULL), initvalArray()
{
}

StmtAST::StmtAST(DataLValIdentAST *lVal_, ExpAST *lOrExp_)
    : st(StmtEnum::STMT_ASSIGN), lVal(lVal_), lOrExp(lOrExp_), mainStmt(NULL), elseStmt(NULL),
      initvalArray()
{
}

StmtAST::StmtAST(DataLValIdentAST *lVal_, std::vector<int> initvalArray_)
    : st(StmtEnum::STMT_ASSIGN_ARRAY), lVal(lVal_), ptr(NULL), mainStmt(NULL), elseStmt(NULL),
      initvalArray(initvalArray_)
{
}

StmtAST::StmtAST(StmtEnum st_, ExpAST *lOrExp_)
    : st(st_), lVal(NULL), lOrExp(lOrExp_), mainStmt(NULL), elseStmt(NULL), initvalArray()
{
}

StmtAST::StmtAST(BlockAST *block_)
    : st(StmtEnum::STMT_BLOCK), lVal(NULL), block(block_), mainStmt(NULL), elseStmt(NULL),
      initvalArray()
{
}

StmtAST::StmtAST(StmtEnum st_, ExpAST *lOrExp_, StmtAST *mainStmt_, StmtAST *elseStmt_)
    : st(st_), lVal(NULL), lOrExp(lOrExp_), mainStmt(mainStmt_), elseStmt(elseStmt_), initvalArray()
{
}

StmtAST::~StmtAST()
{
    if (lVal)
        delete lVal;
    if (st == StmtEnum::STMT_ASSIGN || st == StmtEnum::STMT_EXP || st == StmtEnum::STMT_RET_INT)
        delete lOrExp;
    if (st == StmtEnum::STMT_BLOCK)
        delete block;
    if (mainStmt)
        delete mainStmt;
    if (elseStmt)
        delete elseStmt;
}

const char *StmtAST::getClassName() const { return "StmtAST"; }

void StmtAST::DumpContent(std::ostream &outStream, int indent) const
{
    switch (st)
    {
    case STMT_NONE:
        outStream << Indent(indent + 1) << "NONE" << std::endl;
        break;
    case STMT_EMPTY:
        outStream << Indent(indent + 1) << "STMT_EMPTY" << std::endl;
        break;
    case STMT_ASSIGN:
        outStream << Indent(indent + 1) << "STMT_ASSIGN" << std::endl;
        lVal->Dump(outStream, indent + 1);
        lOrExp->Dump(outStream, indent + 1);
        break;
    case STMT_ASSIGN_ARRAY:
        outStream << Indent(indent + 1) << "STMT_ASSIGN_ARRAY" << std::endl;
        lVal->Dump(outStream, indent + 1);
        outStream << Indent(indent + 1) << "InitvalArray: [";
        for (int elem : initvalArray)
            outStream << elem << ", ";
        outStream << "]" << std::endl;
        break;
    case STMT_EXP:
        outStream << Indent(indent + 1) << "STMT_EXP" << std::endl;
        lOrExp->Dump(outStream, indent + 1);
        break;
    case STMT_RET_INT:
        outStream << Indent(indent + 1) << "STMT_RET_INT" << std::endl;
        lOrExp->Dump(outStream, indent + 1);
        break;
    case STMT_RET_VOID:
        outStream << Indent(indent + 1) << "STMT_RET_VOID" << std::endl;
        break;
    case STMT_BLOCK:
        outStream << Indent(indent + 1) << "STMT_BLOCK" << std::endl;
        block->Dump(outStream, indent + 1);
        break;
    case STMT_IF:
        outStream << Indent(indent + 1) << "STMT_IF" << std::endl;
        lOrExp->Dump(outStream, indent + 1);
        mainStmt->Dump(outStream, indent + 1);
        break;
    case STMT_IF_ELSE:
        outStream << Indent(indent + 1) << "STMT_IF_ELSE" << std::endl;
        lOrExp->Dump(outStream, indent + 1);
        mainStmt->Dump(outStream, indent + 1);
        elseStmt->Dump(outStream, indent + 1);
        break;
    case STMT_WHILE:
        outStream << Indent(indent + 1) << "STMT_WHILE" << std::endl;
        lOrExp->Dump(outStream, indent + 1);
        mainStmt->Dump(outStream, indent + 1);
        break;
    case STMT_BREAK:
        outStream << Indent(indent + 1) << "STMT_BREAK" << std::endl;
        break;
    case STMT_CONT:
        outStream << Indent(indent + 1) << "STMT_CONT" << std::endl;
        break;
    default:
        assert(false);
        break;
    }
}

void StmtAST::setSymbolTable(SymbolTable *symTab)
{
    switch (st)
    {
    case STMT_NONE:
        break;
    case STMT_EMPTY:
        break;
    case STMT_ASSIGN:
        lVal->setSymbolTable(symTab);
        lOrExp->setSymbolTable(symTab);
        break;
    case STMT_ASSIGN_ARRAY:
        lVal->setSymbolTable(symTab);
        break;
    case STMT_EXP:
        lOrExp->setSymbolTable(symTab);
        break;
    case STMT_RET_INT:
        lOrExp->setSymbolTable(symTab);
        break;
    case STMT_RET_VOID:
        break;
    case STMT_BLOCK:
        block->setSymbolTable(symTab);
        break;
    case STMT_IF:
        lOrExp->setSymbolTable(symTab);
        mainStmt->setSymbolTable(symTab);
        break;
    case STMT_IF_ELSE:
        lOrExp->setSymbolTable(symTab);
        mainStmt->setSymbolTable(symTab);
        elseStmt->setSymbolTable(symTab);
        break;
    case STMT_WHILE:
        lOrExp->setSymbolTable(symTab);
        mainStmt->setSymbolTable(symTab);
        break;
    case STMT_BREAK:
        break;
    case STMT_CONT:
        break;
    default:
        assert(false);
        break;
    }
}

void StmtAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab)
{
    switch (st)
    {
    case STMT_NONE:
    {
    }
    break;
    case STMT_EMPTY:
    {
    }
    break;
    case STMT_ASSIGN:
    {
        std::string rValStr = lOrExp->buildIRRetString(irBuilder, symTab);
        std::string lValStr = lVal->buildIRRetAddr(irBuilder, symTab);
        std::string stmt = std::string("store ") + rValStr + std::string(", ") + lValStr;
        irBuilder->pushStmt(stmt);
    }
    break;
    case STMT_ASSIGN_ARRAY:
    {
        std::string rValStr =
            irBuilder->aggregate1DtoNDString(initvalArray, lVal->relaSym->arrayDimVec);
        // std::string lValStr = lVal->buildIRRetAddr(irBuilder, symTab);
        std::string stmt =
            std::string("store ") + rValStr + std::string(", ") + lVal->relaSym->getIRVarName();
        irBuilder->pushStmt(stmt);
    }
    break;
    case STMT_EXP:
    {
        lOrExp->buildIRRetString(irBuilder, symTab);
    }
    break;
    case STMT_RET_INT:
    {
        std::string retValue = lOrExp->buildIRRetString(irBuilder, symTab);
        std::string stmt = std::string("ret ") + retValue;
        irBuilder->pushStmt(stmt);
        irBuilder->pushAndGetBlock(true);
    }
    break;
    case STMT_RET_VOID:
    {
        std::string stmt = std::string("ret");
        irBuilder->pushStmt(stmt);
        irBuilder->pushAndGetBlock(true);
    }
    break;
    case STMT_BLOCK:
    {
        block->buildIR(irBuilder, symTab);
    }
    break;
    case STMT_IF:
    {
        std::string cond = lOrExp->buildIRRetString(irBuilder, symTab);
        IRBlock *entryBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        std::string thenName = irBuilder->currentBlock->blockName;

        mainStmt->buildIR(irBuilder, symTab);
        IRBlock *thenBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        std::string endName = irBuilder->currentBlock->blockName;

        irBuilder->connectIf(cond, entryBlock, thenName, thenBlock, endName);
    }
    break;
    case STMT_IF_ELSE:
    {
        std::string cond = lOrExp->buildIRRetString(irBuilder, symTab);
        IRBlock *entryBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        std::string thenName = irBuilder->currentBlock->blockName;

        mainStmt->buildIR(irBuilder, symTab);
        IRBlock *thenBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        std::string elseName = irBuilder->currentBlock->blockName;

        elseStmt->buildIR(irBuilder, symTab);
        IRBlock *elseBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        std::string endName = irBuilder->currentBlock->blockName;

        irBuilder->connectIfElse(cond, entryBlock, thenName, thenBlock, elseName, elseBlock,
                                 endName);
    }
    break;
    case STMT_WHILE:
    {
        IRBlock *entryBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        std::string testName = irBuilder->currentBlock->blockName;

        std::string cond = lOrExp->buildIRRetString(irBuilder, symTab);
        IRBlock *testBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        std::string loopName = irBuilder->currentBlock->blockName;

        IRBlock *endBlock = irBuilder->getNewBlock();
        std::string endName = endBlock->blockName;

        /*保存上层while的Name*/
        std::string savedTestBlockName = irBuilder->whileTestBlockName;
        std::string savedEndBlockName = irBuilder->whileEndBlockName;
        irBuilder->whileTestBlockName = testName;
        irBuilder->whileEndBlockName = endName;
        /*保存上层while的Name*/

        mainStmt->buildIR(irBuilder, symTab);

        /*恢复上层while的Name*/
        irBuilder->whileTestBlockName = savedTestBlockName;
        irBuilder->whileEndBlockName = savedEndBlockName;
        /*恢复上层while的Name*/

        IRBlock *loopBlock = irBuilder->currentBlock;

        irBuilder->connectWhile(cond, entryBlock, testName, testBlock, loopName, loopBlock,
                                endName);
        irBuilder->pushCurrentBlock();
        irBuilder->setCurrentBlock(endBlock);
    }
    break;
    case STMT_BREAK:
    {
        std::string stmt = std::string("jump ") + irBuilder->whileEndBlockName;
        irBuilder->pushStmt(stmt);
        irBuilder->pushAndGetBlock(true);
    }
    break;
    case STMT_CONT:
    {
        std::string stmt = std::string("jump ") + irBuilder->whileTestBlockName;
        irBuilder->pushStmt(stmt);
        irBuilder->pushAndGetBlock(true);
    }
    break;
    default:
        assert(false);
        break;
    }
}

/* ExpAST */

ExpAST::ExpAST() : opt(OpEnum::OP_NONE), leftExp(NULL), ptr(NULL) {}

ExpAST::ExpAST(PrimaryExpAST *primaryExp_)
    : opt(OpEnum::OP_PRI), leftExp(NULL), primaryExp(primaryExp_)
{
}

ExpAST::ExpAST(OpEnum opt_, ExpAST *rightExp_) : opt(opt_), leftExp(NULL), rightExp(rightExp_)
{
    if (opt == OpEnum::OP_ADD)
        opt = OpEnum::OP_POS;
    if (opt == OpEnum::OP_SUB)
        opt = OpEnum::OP_NEG;
}

ExpAST::ExpAST(OpEnum opt_, ExpAST *leftExp_, ExpAST *rightExp_)
    : opt(opt_), leftExp(leftExp_), rightExp(rightExp_)
{
}

ExpAST::~ExpAST()
{
    if (opt == OpEnum::OP_PRI)
        delete primaryExp;
    else
    {
        if (leftExp)
            delete leftExp;
        if (rightExp)
            delete rightExp;
    }
}

const char *ExpAST::getClassName() const { return "ExpAST"; }

void ExpAST::DumpContent(std::ostream &outStream, int indent) const
{
    if (opt == OpEnum::OP_NONE)
    {
        outStream << Indent(indent + 1) << "NONE" << std::endl;
    }
    else if (opt == OpEnum::OP_PRI)
    {
        outStream << Indent(indent + 1) << optName[(int)(opt)] << std::endl;
        primaryExp->Dump(outStream, indent + 1);
    }
    else
    {
        outStream << Indent(indent + 1) << optName[(int)(opt)] << std::endl;
        if (leftExp)
            leftExp->Dump(outStream, indent + 1);
        rightExp->Dump(outStream, indent + 1);
    }
}

void ExpAST::setSymbolTable(SymbolTable *symTab)
{
    if (opt == OpEnum::OP_NONE)
    {
        assert(false);
    }
    else if (opt == OpEnum::OP_PRI)
    {
        primaryExp->setSymbolTable(symTab);
    }
    else
    {
        if (leftExp)
            leftExp->setSymbolTable(symTab);
        rightExp->setSymbolTable(symTab);
    }
}

int ExpAST::forceCalc(SymbolTable *symTab)
{
    int valLeft, valRight, valRes;

    switch (opt)
    {
    case OpEnum::OP_PRI:
        valRight = primaryExp->forceCalc(symTab);
        valLeft = 0;
        delete primaryExp;
        break;
    case OpEnum::OP_POS:
    case OpEnum::OP_NEG:
    case OpEnum::OP_NOT_B:
    case OpEnum::OP_NOT_L:
        valRight = rightExp->forceCalc(symTab);
        delete rightExp;
        valLeft = 0;
        break;
    case OpEnum::OP_E:
    case OpEnum::OP_NE:
    case OpEnum::OP_L:
    case OpEnum::OP_LE:
    case OpEnum::OP_G:
    case OpEnum::OP_GE:
    case OpEnum::OP_ADD:
    case OpEnum::OP_SUB:
    case OpEnum::OP_MUL:
    case OpEnum::OP_DIV:
    case OpEnum::OP_MOD:
    case OpEnum::OP_AND_B:
    case OpEnum::OP_OR_B:
    case OpEnum::OP_XOR_B:
    case OpEnum::OP_AND_L:
    case OpEnum::OP_OR_L:
        valLeft = leftExp->forceCalc(symTab);
        valRight = rightExp->forceCalc(symTab);
        delete leftExp;
        delete rightExp;
        break;
    default:
        assert(false);
        break;
    }

    switch (opt)
    {
    case OpEnum::OP_PRI:
        valRes = valRight;
        break;
    case OpEnum::OP_POS:
        valRes = valRight;
        break;
    case OpEnum::OP_NEG:
        valRes = -valRight;
        break;
    case OpEnum::OP_NOT_B:
        valRes = ~valRight;
        break;
    case OpEnum::OP_NOT_L:
        valRes = !valRight;
        break;
    case OpEnum::OP_E:
        valRes = valLeft == valRight;
        break;
    case OpEnum::OP_NE:
        valRes = valLeft != valRight;
        break;
    case OpEnum::OP_L:
        valRes = valLeft < valRight;
        break;
    case OpEnum::OP_LE:
        valRes = valLeft <= valRight;
        break;
    case OpEnum::OP_G:
        valRes = valLeft > valRight;
        break;
    case OpEnum::OP_GE:
        valRes = valLeft >= valRight;
        break;
    case OpEnum::OP_ADD:
        valRes = valLeft + valRight;
        break;
    case OpEnum::OP_SUB:
        valRes = valLeft - valRight;
        break;
    case OpEnum::OP_MUL:
        valRes = valLeft * valRight;
        break;
    case OpEnum::OP_DIV:
        valRes = valLeft / valRight;
        break;
    case OpEnum::OP_MOD:
        valRes = valLeft % valRight;
        break;
    case OpEnum::OP_AND_B:
        valRes = valLeft & valRight;
        break;
    case OpEnum::OP_OR_B:
        valRes = valLeft | valRight;
        break;
    case OpEnum::OP_XOR_B:
        valRes = valLeft ^ valRight;
        break;
    case OpEnum::OP_AND_L:
        valRes = valLeft && valRight;
        break;
    case OpEnum::OP_OR_L:
        valRes = valLeft || valRight;
        break;
    default:
        valRes = 0;
        assert(false);
        break;
    }

    opt = OpEnum::OP_PRI;
    leftExp = NULL;
    primaryExp = new PrimaryExpAST(valRes);

    return valRes;
}

void ExpAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab) {}

std::string ExpAST::buildIRRetString(IRBuilder *irBuilder, SymbolTable *symTab)
{
    std::string left, left2, right, right2, resPtr, res, stmt;
    IRBlock *entryBlock, *thenBlock;
    std::string thenName, endName;

    switch (opt)
    {
    case OpEnum::OP_PRI:
        res = primaryExp->buildIRRetString(irBuilder, symTab);
        break;
    case OpEnum::OP_POS:
        res = rightExp->buildIRRetString(irBuilder, symTab);
        break;
    case OpEnum::OP_NEG:
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = sub 0, ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_NOT_B:
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = xor -1, ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_NOT_L:
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = eq 0, ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_E:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = eq ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_NE:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = ne ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_L:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = lt ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_LE:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = le ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_G:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = gt ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_GE:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = ge ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_ADD:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = add ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_SUB:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = sub ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_MUL:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = mul ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_DIV:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = div ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_MOD:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = mod ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_AND_B:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = and ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_OR_B:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = or ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_XOR_B:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        right = rightExp->buildIRRetString(irBuilder, symTab);
        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = xor ") + left + std::string(", ") + right;
        irBuilder->pushStmt(stmt);
        break;
    case OpEnum::OP_AND_L:
        left = leftExp->buildIRRetString(irBuilder, symTab);
        resPtr = irBuilder->getNextIdent();
        stmt = resPtr + std::string(" = alloc i32");
        irBuilder->pushStmt(stmt);
        stmt = std::string("store 0, ") + resPtr;
        irBuilder->pushStmt(stmt);
        entryBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        thenName = irBuilder->currentBlock->blockName;

        right2 = rightExp->buildIRRetString(irBuilder, symTab);
        right = irBuilder->getNextIdent();
        stmt = right + std::string(" = ne 0, ") + right2;
        irBuilder->pushStmt(stmt);
        stmt = std::string("store ") + right + std::string(", ") + resPtr;
        irBuilder->pushStmt(stmt);
        thenBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        endName = irBuilder->currentBlock->blockName;

        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = load ") + resPtr;
        irBuilder->pushStmt(stmt);

        irBuilder->connectIf(left, entryBlock, thenName, thenBlock, endName);
        break;
    case OpEnum::OP_OR_L:
        /*TODO 其他实现？*/
        left2 = leftExp->buildIRRetString(irBuilder, symTab);
        left = irBuilder->getNextIdent();
        stmt = left + std::string(" = eq 0, ") + left2;
        irBuilder->pushStmt(stmt);
        resPtr = irBuilder->getNextIdent();
        stmt = resPtr + std::string(" = alloc i32");
        irBuilder->pushStmt(stmt);
        stmt = std::string("store 1, ") + resPtr;
        irBuilder->pushStmt(stmt);
        entryBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        thenName = irBuilder->currentBlock->blockName;

        right2 = rightExp->buildIRRetString(irBuilder, symTab);
        right = irBuilder->getNextIdent();
        stmt = right + std::string(" = ne 0, ") + right2;
        irBuilder->pushStmt(stmt);
        stmt = std::string("store ") + right + std::string(", ") + resPtr;
        irBuilder->pushStmt(stmt);
        thenBlock = irBuilder->currentBlock;

        irBuilder->pushAndGetBlock();
        endName = irBuilder->currentBlock->blockName;

        res = irBuilder->getNextIdent();
        stmt = res + std::string(" = load ") + resPtr;
        irBuilder->pushStmt(stmt);

        irBuilder->connectIf(left, entryBlock, thenName, thenBlock, endName);
        break;
    default:
        assert(false);
        break;
    }

    return res;
}

/* PrimaryExpAST */

PrimaryExpAST::PrimaryExpAST() : type(PrimEnum::PRI_NONE), constVal(0), funcName(), ptr(NULL) {}

PrimaryExpAST::PrimaryExpAST(DataLValIdentAST *lVal_)
    : type(PrimEnum::PRI_LVAL), constVal(0), funcName(), lVal(lVal_)
{
}

PrimaryExpAST::PrimaryExpAST(int constVal_)
    : type(PrimEnum::PRI_CONST), constVal(constVal_), funcName(), ptr(NULL)
{
}

PrimaryExpAST::PrimaryExpAST(std::string funcName_, FuncRParamsAST *paras_)
    : type(PrimEnum::PRI_CALL), constVal(0), funcName(funcName_), paras(paras_)
{
}

PrimaryExpAST::PrimaryExpAST(const char *funcName_, FuncRParamsAST *paras_)
    : type(PrimEnum::PRI_CALL), constVal(0), funcName(funcName_), paras(paras_)
{
}

PrimaryExpAST::~PrimaryExpAST()
{
    if (type == PrimEnum::PRI_LVAL)
        delete lVal;
    if (type == PrimEnum::PRI_CALL)
        delete paras;
}

const char *PrimaryExpAST::getClassName() const { return "PrimaryExpAST"; }

void PrimaryExpAST::DumpContent(std::ostream &outStream, int indent) const
{
    if (type == PrimEnum::PRI_NONE)
        outStream << Indent(indent + 1) << "NONE" << std::endl;
    if (type == PrimEnum::PRI_CONST)
        outStream << Indent(indent + 1) << "Number: " << constVal << std::endl;
    if (type == PrimEnum::PRI_LVAL)
        lVal->Dump(outStream, indent + 1);
    if (type == PrimEnum::PRI_CALL)
    {
        outStream << Indent(indent + 1) << "FuncName: " << funcName << std::endl;
        paras->Dump(outStream, indent + 1);
    }
}

void PrimaryExpAST::setSymbolTable(SymbolTable *symTab)
{

    if (type == PrimEnum::PRI_NONE)
        assert(false);
    if (type == PrimEnum::PRI_CONST)
        ;
    if (type == PrimEnum::PRI_LVAL)
        lVal->setSymbolTable(symTab);
    if (type == PrimEnum::PRI_CALL)
        paras->setSymbolTable(symTab);
}

int PrimaryExpAST::forceCalc(SymbolTable *symTab)
{
    if (type == PrimEnum::PRI_CONST)
    {
        return constVal;
    }
    if (type == PrimEnum::PRI_LVAL)
    {
        if (!lVal->relaSym)
            lVal->setSymbolTable(symTab);
        return lVal->relaSym->initval;
    }
    assert(true);
    return 0;
}

void PrimaryExpAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab) {}

std::string PrimaryExpAST::buildIRRetString(IRBuilder *irBuilder, SymbolTable *symTab)
{
    std::string res, params, stmt;
    switch (type)
    {
    case PrimEnum::PRI_CONST:
        res = std::to_string(constVal);
        break;
    case PrimEnum::PRI_LVAL:
        res = lVal->buildIRRetValue(irBuilder, symTab);
        break;
    case PrimEnum::PRI_CALL:
        params = std::string("(") + paras->buildIRRetString(irBuilder, symTab) + ")";
        res = irBuilder->getNextIdent();
        if (symTab->funcRetVoid(funcName))
            stmt = std::string("call @") + funcName + params;
        else
            stmt = res + std::string(" = call @") + funcName + params;
        irBuilder->pushStmt(stmt);
        break;
    default:
        assert(false);
        break;
    }
    return res;
}

/* DataDeclAST */

DataDeclAST::DataDeclAST() : defi(DefiEnum::DEFI_NONE), type(TypeEnum::TYPE_NONE), defVec() {}

DataDeclAST::DataDeclAST(DefiEnum defi_, TypeEnum type_, DataDefAST *def_)
    : defi(defi_), type(type_), defVec()
{
    if (def_)
        append(def_);
}

DataDeclAST::~DataDeclAST()
{
    for (DataDefAST *def : defVec)
        delete def;
}

void DataDeclAST::append(DataDefAST *def_)
{
    def_->defi = defi;
    def_->type = type;
    if (def_)
        defVec.push_back(def_);
}

const char *DataDeclAST::getClassName() const { return "DataDeclAST"; }

void DataDeclAST::DumpContent(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent + 1) << "Defi: " << defiName[(int)(defi)] << std::endl;
    outStream << Indent(indent + 1) << "Type: " << typeName[(int)(type)] << std::endl;
    for (DataDefAST *def : defVec)
        def->Dump(outStream, indent + 1);
}

void DataDeclAST::setSymbolTable(SymbolTable *symTab)
{
    for (DataDefAST *def : defVec)
        def->setSymbolTable(symTab);
}

void DataDeclAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab) {}

/* DataDefAST */

DataDefAST::DataDefAST()
    : defi(DefiEnum::DEFI_NONE), type(TypeEnum::TYPE_NONE), defIdent(NULL), initval(NULL),
      stmtAfterSym(NULL)
{
}

DataDefAST::DataDefAST(DefiEnum defi_, TypeEnum type_, DataLValIdentAST *defIdent_,
                       DataInitvalAST *initval_)
    : defi(defi_), type(type_), defIdent(defIdent_), initval(initval_), stmtAfterSym(NULL)
{
}

const char *DataDefAST::getClassName() const { return "DataDefAST"; }

DataDefAST::~DataDefAST()
{
    if (defIdent)
        delete defIdent;
    if (initval)
        delete initval;
    assert(stmtAfterSym == NULL);
}

void DataDefAST::DumpContent(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent + 1) << "Defi: " << defiName[(int)(defi)] << std::endl;
    outStream << Indent(indent + 1) << "Type: " << typeName[(int)(type)] << std::endl;
    if (defIdent)
        defIdent->Dump(outStream, indent + 1);
    if (initval)
        initval->Dump(outStream, indent + 1);
    if (stmtAfterSym)
        stmtAfterSym->Dump(outStream, indent + 1);
}

void DataDefAST::setSymbolTable(SymbolTable *symTab)
{
    /*全局变量：放入符号表中，符号表有初值，不设置stmtAfterSym*/
    /*全局常量：放入符号表中，符号表有初值，不设置stmtAfterSym*/
    /*全局数组：放入符号表中，符号表有初值，不设置stmtAfterSym*/
    /*全局常组：放入符号表中，符号表有初值，不设置stmtAfterSym*/
    /*局部变量：放入符号表中，符号表没有初值，设置stmtAfterSym*/
    /*局部常量：放入符号表中，符号表有初值，不设置stmtAfterSym*/
    /*局部数组：放入符号表中，符号表没有初值，设置stmtAfterSym*/
    /*局部常组：放入符号表中，符号表有初值，不设置stmtAfterSym*/

    std::vector<int> arrayDimVec_ = defIdent->getArrayDim(symTab);
    std::string ident_ = defIdent->ident;
    int arrayDimCum = 1;
    for (int e : arrayDimVec_)
        arrayDimCum *= e;

    int initval_ = 0;
    std::vector<int> initvalArray_;
    bool initInSymTab =
        ((symTab->currentBlockVecIndex.size() == 0) || (defi == DefiEnum::DEFI_CONST));
    bool setStmtAfterSym = (initval != NULL && !initInSymTab);

    if (arrayDimVec_.size() == 0 && initInSymTab)
    {
        /*全局变量，全局常量，局部常量*/
        if (initval != NULL)
            initval_ = initval->exp->forceCalc(symTab);
    }

    if (arrayDimVec_.size() != 0 && initInSymTab)
    {
        /*全局数组，全局常组，局部常组*/
        if (initval != NULL)
            initvalArray_ = initval->getInitVector(arrayDimVec_);
        else
            initvalArray_ = std::vector<int>(arrayDimCum);
    }

    if (arrayDimVec_.size() == 0 && setStmtAfterSym)
    {
        /*局部变量*/
        stmtAfterSym = new StmtAST(defIdent, initval->exp);
        initval->exp = NULL;
        defIdent = NULL;
    }

    if (arrayDimVec_.size() != 0 && setStmtAfterSym)
    {
        /*局部数组*/
        for (ExpAST *&exp_ : defIdent->expVec)
            delete exp_;
        defIdent->expVec.clear();
        std::vector<int> initvalArray__ = initval->getInitVector(arrayDimVec_);
        stmtAfterSym = new StmtAST(defIdent, initvalArray__);
        defIdent = NULL;
    }

    SymbolEntry *sym = new SymbolEntry(symTab, type, defi, symTab->currentFuncName,
                                       symTab->currentBlockVecIndex, symTab->currentBlockLineIndex,
                                       ident_, arrayDimVec_, initval_, initvalArray_, false);
    symTab->append(sym);
}

void DataDefAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab) {}

/* FuncFParamsAST */

FuncFParamsAST::FuncFParamsAST() : paraVec() {}

FuncFParamsAST::FuncFParamsAST(FuncFParamAST *para_) : paraVec()
{
    if (para_)
        append(para_);
}

FuncFParamsAST::~FuncFParamsAST()
{
    for (FuncFParamAST *para : paraVec)
        delete para;
}

void FuncFParamsAST::append(FuncFParamAST *para_)
{
    if (para_)
        paraVec.push_back(para_);
}

void FuncFParamsAST::DumpContent(std::ostream &outStream, int indent) const
{
    for (FuncFParamAST *para : paraVec)
        para->Dump(outStream, indent + 1);
}

const char *FuncFParamsAST::getClassName() const { return "FuncFParamsAST"; }

void FuncFParamsAST::setSymbolTable(SymbolTable *symTab)
{
    symTab->enterBlock();
    for (FuncFParamAST *para : paraVec)
        para->setSymbolTable(symTab);
    symTab->antiLeaveBlock();
}

void FuncFParamsAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab) {}

std::string FuncFParamsAST::buildIRRetString(IRBuilder *irBuilder, SymbolTable *symTab)
{
    int count = 0;
    std::string inputType;
    for (FuncFParamAST *para : paraVec)
    {
        if (count++ != 0)
            inputType += ", ";
        inputType += para->buildIRRetString(irBuilder, symTab);
    }
    return inputType;
}

/* FuncFParamAST */

FuncFParamAST::FuncFParamAST() : type(TypeEnum::TYPE_NONE), para(NULL) {}

FuncFParamAST::FuncFParamAST(TypeEnum type_, DataLValIdentAST *para_) : type(type_), para(para_) {}

FuncFParamAST::~FuncFParamAST() {}

void FuncFParamAST::DumpContent(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent + 1) << "Type: " << typeName[(int)(type)] << std::endl;
    para->Dump(outStream, indent + 1);
}

const char *FuncFParamAST::getClassName() const { return "FuncFParamAST"; }

void FuncFParamAST::setSymbolTable(SymbolTable *symTab)
{
    SymbolEntry *sym =
        new SymbolEntry(symTab, para->type, para->defi, symTab->currentFuncName,
                        symTab->currentBlockVecIndex, symTab->currentBlockLineIndex, para->ident,
                        para->getArrayDim(), 0, std::vector<int>(), true);
    para->relaSym = sym;
    symTab->append(sym);
}

void FuncFParamAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab) {}

std::string FuncFParamAST::buildIRRetString(IRBuilder *irBuilder, SymbolTable *symTab)
{
    std::string irName = para->relaSym->getIRVarName(true);
    std::string typeName = irBuilder->getIRType(para->relaSym->arrayDimVec);
    return irName + ": " + typeName;
}

/* FuncRParamsAST */

FuncRParamsAST::FuncRParamsAST() : expVec() {}

FuncRParamsAST::FuncRParamsAST(ExpAST *exp_) : expVec()
{
    if (exp_)
        append(exp_);
}

FuncRParamsAST::~FuncRParamsAST()
{
    for (ExpAST *exp : expVec)
        delete exp;
}

void FuncRParamsAST::append(ExpAST *exp_)
{
    if (exp_)
        expVec.push_back(exp_);
}

void FuncRParamsAST::DumpContent(std::ostream &outStream, int indent) const
{
    for (ExpAST *exp : expVec)
        exp->Dump(outStream, indent + 1);
}

const char *FuncRParamsAST::getClassName() const { return "FuncRParamsAST"; }

void FuncRParamsAST::setSymbolTable(SymbolTable *symTab)
{
    for (ExpAST *exp : expVec)
        exp->setSymbolTable(symTab);
}

void FuncRParamsAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab) {}

std::string FuncRParamsAST::buildIRRetString(IRBuilder *irBuilder, SymbolTable *symTab)
{
    int count = 0;
    std::string inputType;
    for (ExpAST *exp : expVec)
    {
        if (count++ != 0)
            inputType += ", ";
        inputType += exp->buildIRRetString(irBuilder, symTab);
    }
    return inputType;
}

/* DataLValIdentAST */

DataLValIdentAST::DataLValIdentAST()
    : defi(DefiEnum::DEFI_NONE), type(TypeEnum::TYPE_NONE), ident(), emptyValStart(false), expVec()
{
}

DataLValIdentAST::DataLValIdentAST(DefiEnum defi_, TypeEnum type_, std::string ident_,
                                   bool emptyValStart)
    : defi(defi_), type(type_), ident(ident_), emptyValStart(emptyValStart), expVec()
{
}

DataLValIdentAST::DataLValIdentAST(DefiEnum defi_, TypeEnum type_, const char *ident_,
                                   bool emptyValStart)
    : defi(defi_), type(type_), ident(ident_), emptyValStart(emptyValStart), expVec()
{
}

DataLValIdentAST::~DataLValIdentAST()
{
    for (ExpAST *exp : expVec)
        delete exp;
}

void DataLValIdentAST::append(ExpAST *exp_)
{
    if (exp_)
        expVec.push_back(exp_);
}

void DataLValIdentAST::DumpContent(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent + 1) << "Defi: " << defiName[(int)(defi)] << std::endl;
    outStream << Indent(indent + 1) << "Type: " << typeName[(int)(type)] << std::endl;
    outStream << Indent(indent + 1) << "Ident: " << ident << std::endl;
    outStream << Indent(indent + 1) << "emptyValStart: " << (emptyValStart ? "YES" : "NO")
              << std::endl;
    for (ExpAST *exp : expVec)
        exp->Dump(outStream, indent + 1);
}

const char *DataLValIdentAST::getClassName() const { return "DataLValIdentAST"; }

void DataLValIdentAST::setSymbolTable(SymbolTable *symTab)
{
    relaSym = symTab->match(ident, type, defi);
}

std::vector<int> DataLValIdentAST::getArrayDim(SymbolTable *symTab)
{
    std::vector<int> arrayDimVec;
    if (emptyValStart)
        arrayDimVec.push_back(-1);
    for (ExpAST *&exp_ : expVec)
    {
        int expval = exp_->forceCalc(symTab);
        arrayDimVec.push_back(expval);
    }
    return arrayDimVec;
}

void DataLValIdentAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab) {}

std::string DataLValIdentAST::buildIRRetValue(IRBuilder *irBuilder, SymbolTable *symTab)
{
    if (!relaSym)
        setSymbolTable(symTab);
    if (!relaSym->isArray())
    {
        /*局部变量，全局变量*/
        if (relaSym->defi == DefiEnum::DEFI_VAR)
        {
            std::string res = irBuilder->getNextIdent();
            std::string stmt = res + std::string(" = load ") + relaSym->getIRVarName();
            irBuilder->pushStmt(stmt);
            return res;
        }
        /*局部常量，全局常量*/
        else if (relaSym->defi == DefiEnum::DEFI_CONST)
            return std::to_string(relaSym->initval);
        else
            assert(false);
    }
    else
    {
        std::string addr = buildIRRetAddr(irBuilder, symTab);
        std::string res = irBuilder->getNextIdent();
        std::string stmt = res + " = load " + addr;
        irBuilder->pushStmt(stmt);
        return res;
    }
}

std::string DataLValIdentAST::buildIRRetAddr(IRBuilder *irBuilder, SymbolTable *symTab)
{
    if (!relaSym)
        setSymbolTable(symTab);
    if (!relaSym->isArray())
    {
        /*局部变量，全局变量*/
        if (relaSym->defi == DefiEnum::DEFI_VAR)
            return relaSym->getIRVarName();
        /*局部常量，全局常量*/
        else if (relaSym->defi == DefiEnum::DEFI_CONST)
            assert(false);
        else
            assert(false);
    }
    else
    {
        std::string last = relaSym->getIRVarName(), next, stmt;
        for (auto exp : expVec)
        {
            std::string expRes = exp->buildIRRetString(irBuilder, symTab);
            next = irBuilder->getNextIdent();
            stmt = next + " = getelemptr " + last + ", " + expRes;
            last = next;
            irBuilder->pushStmt(stmt);
        }
        return last;
    }
}

/* DataInitvalAST */

DataInitvalAST::DataInitvalAST()
    : defi(DefiEnum::DEFI_NONE), type(TypeEnum::TYPE_NONE), exp(NULL), initVec()
{
}

DataInitvalAST::DataInitvalAST(DefiEnum defi_, TypeEnum type_, ExpAST *exp_)
    : defi(defi_), type(type_), exp(exp_), initVec()
{
}

DataInitvalAST::DataInitvalAST(DefiEnum defi_, TypeEnum type_, DataInitvalAST *initVal_)
    : defi(defi_), type(type_), exp(NULL), initVec()
{
    if (initVal_)
        initVec.push_back(initVal_);
}

DataInitvalAST::~DataInitvalAST()
{
    if (exp)
        delete exp;
    for (DataInitvalAST *elem : initVec)
        delete elem;
}

void DataInitvalAST::append(DataInitvalAST *initVal_)
{
    if (initVal_)
        initVec.push_back(initVal_);
}

void DataInitvalAST::DumpContent(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent + 1) << "Defi: " << defiName[(int)(defi)] << std::endl;
    outStream << Indent(indent + 1) << "Type: " << typeName[(int)(type)] << std::endl;
    if (exp)
        exp->Dump(outStream, indent + 1);
    else
    {
        for (DataInitvalAST *initVal : initVec)
            initVal->Dump(outStream, indent + 1);
    }
}

const char *DataInitvalAST::getClassName() const { return "DataInitvalAST"; }

void DataInitvalAST::setSymbolTable(SymbolTable *symTab) {}

std::vector<int> DataInitvalAST::getInitVector(std::vector<int> arrayDim, SymbolTable *symTab)
{
    /* TODO */
    int cum = 1;
    for (auto e : arrayDim)
        cum *= e;
    std::vector<int> vec;
    dfs(vec, symTab);
    int len = vec.size();
    for (int i = len; i < cum; i++)
        vec.push_back(0);
    for (int i = cum; i < len; i++)
        vec.pop_back();
    return vec;
}

void DataInitvalAST::dfs(std::vector<int> &vec, SymbolTable *symTab)
{
    if (exp)
        vec.push_back(exp->forceCalc(symTab));
    else
        for (DataInitvalAST *e : initVec)
            e->dfs(vec, symTab);
}

void DataInitvalAST::buildIR(IRBuilder *irBuilder, SymbolTable *symTab) {}

/* END */
