#include "ast.hpp"
#include "define.hpp"
#include "symtab.hpp"
#include <assert.h>
#include <iomanip>

using namespace std;

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
    for (FuncDefAST *func : funcVec)
        func->setSymbolTable(symTab);
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
}

void BlockAST::setSymbolTable(SymbolTable *symTab)
{
    symTab->enterBlock();
    vecIndex = symTab->currentBlockVecIndex;
    for (auto item : itemVec)
        item->setSymbolTable(symTab);
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

void BlockItemAST::setSymbolTable(SymbolTable *symTab) {}

/* StmtAST */

StmtAST::StmtAST() : st(StmtEnum::STMT_NONE), lVal(NULL), ptr(NULL), mainStmt(NULL), elseStmt(NULL)
{
}

StmtAST::StmtAST(StmtEnum st_) : st(st_), lVal(NULL), ptr(NULL), mainStmt(NULL), elseStmt(NULL) {}

StmtAST::StmtAST(DataLValIdentAST *lVal_, ExpAST *lOrExp_)
    : st(StmtEnum::STMT_ASSIGN), lVal(lVal_), lOrExp(lOrExp_), mainStmt(NULL), elseStmt(NULL)
{
}

StmtAST::StmtAST(StmtEnum st_, ExpAST *lOrExp_)
    : st(st_), lVal(NULL), lOrExp(lOrExp_), mainStmt(NULL), elseStmt(NULL)
{
}

StmtAST::StmtAST(BlockAST *block_)
    : st(StmtEnum::STMT_BLOCK), lVal(NULL), block(block_), mainStmt(NULL), elseStmt(NULL)
{
}

StmtAST::StmtAST(StmtEnum st_, ExpAST *lOrExp_, StmtAST *mainStmt_, StmtAST *elseStmt_)
    : st(st_), lVal(NULL), lOrExp(lOrExp_), mainStmt(mainStmt_), elseStmt(elseStmt_)
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
    case STMT_EMPTY:;
        outStream << Indent(indent + 1) << "STMT_EMPTY" << std::endl;
        break;
    case STMT_ASSIGN:;
        outStream << Indent(indent + 1) << "STMT_ASSIGN" << std::endl;
        lVal->Dump(outStream, indent + 1);
        lOrExp->Dump(outStream, indent + 1);
        break;
    case STMT_EXP:;
        outStream << Indent(indent + 1) << "STMT_EXP" << std::endl;
        lOrExp->Dump(outStream, indent + 1);
        break;
    case STMT_RET_INT:;
        outStream << Indent(indent + 1) << "STMT_RET_INT" << std::endl;
        lOrExp->Dump(outStream, indent + 1);
        break;
    case STMT_RET_VOID:;
        outStream << Indent(indent + 1) << "STMT_RET_VOID" << std::endl;
        break;
    case STMT_BLOCK:;
        outStream << Indent(indent + 1) << "STMT_BLOCK" << std::endl;
        block->Dump(outStream, indent + 1);
        break;
    case STMT_IF:;
        outStream << Indent(indent + 1) << "STMT_IF" << std::endl;
        lOrExp->Dump(outStream, indent + 1);
        mainStmt->Dump(outStream, indent + 1);
        break;
    case STMT_IF_ELSE:;
        outStream << Indent(indent + 1) << "STMT_IF_ELSE" << std::endl;
        lOrExp->Dump(outStream, indent + 1);
        mainStmt->Dump(outStream, indent + 1);
        elseStmt->Dump(outStream, indent + 1);
        break;
    case STMT_WHILE:;
        outStream << Indent(indent + 1) << "STMT_WHILE" << std::endl;
        lOrExp->Dump(outStream, indent + 1);
        mainStmt->Dump(outStream, indent + 1);
        break;
    case STMT_BREAK:;
        outStream << Indent(indent + 1) << "STMT_BREAK" << std::endl;
        break;
    case STMT_CONT:;
        outStream << Indent(indent + 1) << "STMT_CONT" << std::endl;
        break;
    default:
        break;
    }
}

void StmtAST::setSymbolTable(SymbolTable *symTab) {}

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

void ExpAST::setSymbolTable(SymbolTable *symTab) {}

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
        assert(true);
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
        assert(true);
        break;
    }

    opt = OpEnum::OP_PRI;
    leftExp = NULL;
    primaryExp = new PrimaryExpAST(valRes);

    return valRes;
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
    if (paras)
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

void PrimaryExpAST::setSymbolTable(SymbolTable *symTab) {}

int PrimaryExpAST::forceCalc(SymbolTable *symTab)
{
    if (type == PrimEnum::PRI_CONST)
    {
        return constVal;
    }
    if (type == PrimEnum::PRI_LVAL)
    {
        SymbolEntry *sym = symTab->match(lVal->ident, lVal->type, lVal->defi,
                                         symTab->currentFuncName, symTab->currentBlockVecIndex);
        assert(sym);
        return sym->initval;
    }
    assert(true);
    return 0;
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

/* DataDefAST */

DataDefAST::DataDefAST()
    : defi(DefiEnum::DEFI_NONE), type(TypeEnum::TYPE_NONE), defIdent(NULL), initval(NULL)
{
}

DataDefAST::DataDefAST(DefiEnum defi_, TypeEnum type_, DataLValIdentAST *defIdent_,
                       DataInitvalAST *initval_)
    : defi(defi_), type(type_), defIdent(defIdent_), initval(initval_)
{
}

const char *DataDefAST::getClassName() const { return "DataDefAST"; }

DataDefAST::~DataDefAST()
{
    if (defIdent)
        delete defIdent;
    if (initval)
        delete initval;
}

void DataDefAST::DumpContent(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent + 1) << "Defi: " << defiName[(int)(defi)] << std::endl;
    outStream << Indent(indent + 1) << "Type: " << typeName[(int)(type)] << std::endl;
    if (defIdent)
        defIdent->Dump(outStream, indent + 1);
    if (initval)
        initval->Dump(outStream, indent + 1);
}

void DataDefAST::setSymbolTable(SymbolTable *symTab)
{
    SymbolEntry *sym =
        new SymbolEntry(symTab, type, defi, symTab->currentFuncName, symTab->currentBlockVecIndex,
                        symTab->currentBlockLineIndex, defIdent, initval);
    symTab->append(sym);
}

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
    SymbolEntry *sym = new SymbolEntry(symTab, type, para->defi, symTab->currentFuncName,
                                       symTab->currentBlockVecIndex, symTab->currentBlockLineIndex,
                                       para, NULL, true);
    symTab->append(sym);
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

void FuncRParamsAST::setSymbolTable(SymbolTable *symTab) {}

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

void DataLValIdentAST::setSymbolTable(SymbolTable *symTab) {}

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
