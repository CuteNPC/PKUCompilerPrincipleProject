#include "ast.hpp"
#include <iomanip>

using namespace std;

static const char *defiName[] = {"NONE", "Var", "Const"};
static const char *typeName[] = {"NONE", "void", "int", "const"};
static const char *optName[] = {

    "OP_NONE",

    "OP_PRI",

    "OP_E",     "OP_NE",    "OP_L",    "OP_LE",    "OP_G",   "OP_GE",

    "OP_POS",   "OP_NEG",   "OP_ADD",  "OP_SUB",   "OP_MUL", "OP_DIV", "OP_MOD",

    "OP_NOT_B", "OP_AND_B", "OP_OR_B", "OP_XOR_B",

    "OP_NOT_L", "OP_AND_L", "OP_OR_L",

};

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
    const static uint32_t indentWidth = 4;
    const static char ch = '+';
    if (indentWidth)
    {
        for (int i = 0; i < indent.val; i++)
        {
            outStream << ch;
            for (int j = 1; j < indentWidth; j++)
                outStream << ' ';
        }
    }
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

CompUnitAST::CompUnitAST() : funcDef(NULL){};

CompUnitAST::CompUnitAST(FuncDefAST *funcDef_) : funcDef(funcDef_){};

CompUnitAST::~CompUnitAST()
{
    if (funcDef)
        delete funcDef;
};

const char *CompUnitAST::getClassName() const { return "CompUnitAST"; }

void CompUnitAST::DumpContent(std::ostream &outStream, int indent) const
{
    funcDef->Dump(outStream, indent + 1);
}

/* FuncDefAST */

FuncDefAST::FuncDefAST() : funcType(TypeEnum::TYPE_NONE), funcName(), funcBody(NULL){};

FuncDefAST::FuncDefAST(TypeEnum funcType_, std::string funcName_, BlockAST *funcBody_)
    : funcType(funcType_), funcName(funcName_), funcBody(funcBody_){};

FuncDefAST::FuncDefAST(TypeEnum funcType_, const char *funcName_, BlockAST *funcBody_)
    : funcType(funcType_), funcName(funcName_), funcBody(funcBody_){};

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
    funcBody->Dump(outStream, indent + 1);
}

/* BlockAST */

BlockAST::BlockAST() : itemVec() {}

void BlockAST::append(BlockItemAST *item) { itemVec.push_back(item); }

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

/* StmtAST */

StmtAST::StmtAST() : st(StmtEnum::STMT_NONE), ident(), ptr(NULL), mainStmt(NULL), elseStmt(NULL) {}

StmtAST::StmtAST(StmtEnum st_) : st(st_), ident(), ptr(NULL), mainStmt(NULL), elseStmt(NULL) {}

StmtAST::StmtAST(std::string ident_, ExpAST *lOrExp_)
    : st(StmtEnum::STMT_ASSIGN), ident(ident_), lOrExp(lOrExp_), mainStmt(NULL), elseStmt(NULL)
{
}

StmtAST::StmtAST(const char *ident_, ExpAST *lOrExp_)
    : st(StmtEnum::STMT_ASSIGN), ident(ident_), lOrExp(lOrExp_), mainStmt(NULL), elseStmt(NULL)
{
}

StmtAST::StmtAST(StmtEnum st_, ExpAST *lOrExp_)
    : st(st_), ident(), lOrExp(lOrExp_), mainStmt(NULL), elseStmt(NULL)
{
}

StmtAST::StmtAST(BlockAST *block_)
    : st(StmtEnum::STMT_BLOCK), ident(), block(block_), mainStmt(NULL), elseStmt(NULL)
{
}

StmtAST::StmtAST(StmtEnum st_, ExpAST *lOrExp_, StmtAST *mainStmt_, StmtAST *elseStmt_)
    : st(st_), ident(), lOrExp(lOrExp_), mainStmt(mainStmt_), elseStmt(elseStmt_)
{
}

StmtAST::~StmtAST()
{
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
        outStream << Indent(indent + 1) << "LVal: " << ident << std::endl;
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
    default:
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

/* PrimaryExpAST */

PrimaryExpAST::PrimaryExpAST() : type(PrimEnum::PRI_NONE), constVal(0), lValIdent() {}

PrimaryExpAST::PrimaryExpAST(int constVal_)
    : type(PrimEnum::PRI_CONST), constVal(constVal_), lValIdent()
{
}

PrimaryExpAST::PrimaryExpAST(std::string lValIdent_)
    : type(PrimEnum::PRI_LVAL), constVal(0), lValIdent(lValIdent_)
{
}

PrimaryExpAST::~PrimaryExpAST() {}

const char *PrimaryExpAST::getClassName() const { return "PrimaryExpAST"; }

void PrimaryExpAST::DumpContent(std::ostream &outStream, int indent) const
{
    if (type == PrimEnum::PRI_NONE)
        outStream << Indent(indent + 1) << "NONE" << std::endl;
    if (type == PrimEnum::PRI_CONST)
        outStream << Indent(indent + 1) << "Number: " << constVal << std::endl;
    if (type == PrimEnum::PRI_LVAL)
        outStream << Indent(indent + 1) << "Ident: " << lValIdent << std::endl;
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

/* DataDefAST */

DataDefAST::DataDefAST() : defi(DefiEnum::DEFI_NONE), type(TypeEnum::TYPE_NONE), ident(), exp(NULL)
{
}

DataDefAST::DataDefAST(DefiEnum defi_, std::string ident_, ExpAST *exp_)
    : defi(defi_), type(TypeEnum::TYPE_NONE), ident(ident_), exp(exp_)
{
}

DataDefAST::DataDefAST(DefiEnum defi_, const char *ident_, ExpAST *exp_)
    : defi(defi_), type(TypeEnum::TYPE_NONE), ident(ident_), exp(exp_)
{
}

const char *DataDefAST::getClassName() const { return "DataDefAST"; }

DataDefAST::~DataDefAST()
{
    if (exp)
        delete exp;
}

void DataDefAST::DumpContent(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent + 1) << "Defi: " << defiName[(int)(defi)] << std::endl;
    outStream << Indent(indent + 1) << "Type: " << typeName[(int)(type)] << std::endl;
    outStream << Indent(indent + 1) << "Ident: " << ident << std::endl;
    if (exp)
        exp->Dump(outStream, indent + 1);
}
