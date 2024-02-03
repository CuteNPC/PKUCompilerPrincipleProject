#include "ast.hpp"
#include <iomanip>

static const char *typeName[] = {"", "void", "int", "const"};
static const char *optName[] = {

    "",

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
    Indent(uint32_t num = 0) : val(num){};
    friend std::ostream &operator<<(std::ostream &outStream, const Indent &ast);
};

std::ostream &operator<<(std::ostream &outStream, const Indent &indent)
{
    const static int indentWidth = 2;
    if (indent.val)
        outStream << std::setw(indent.val * indentWidth) << ' ';
    return outStream;
}

std::ostream &operator<<(std::ostream &outStream, const BaseAST &ast)
{
    ast.Dump(outStream, 0);
    return outStream;
}

CompUnitAST::CompUnitAST() : funcDef(NULL){};

CompUnitAST::CompUnitAST(FuncDefAST *funcDef_) : funcDef(funcDef_){};

CompUnitAST::~CompUnitAST()
{
    if (funcDef)
        delete funcDef;
};

void CompUnitAST::Dump(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent) << "CompUnitAST {" << std::endl;
    funcDef->Dump(outStream, indent + 1);
    outStream << Indent(indent) << "}" << std::endl;
}

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

void FuncDefAST ::Dump(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent) << "FuncDefAST { " << std::endl;
    outStream << Indent(indent + 1) << "funcType: " << typeName[(int)(funcType)] << std::endl;
    outStream << Indent(indent + 1) << "funcName: " << funcName << std::endl;
    funcBody->Dump(outStream, indent + 1);
    outStream << Indent(indent) << "}" << std::endl;
}

BlockAST ::BlockAST() : stmt(NULL) {}

BlockAST ::BlockAST(StmtAST *stmt_) : stmt(stmt_) {}

BlockAST ::~BlockAST()
{
    if (stmt)
        delete stmt;
}

void BlockAST ::Dump(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent) << "BlockAST { " << std::endl;
    stmt->Dump(outStream, indent + 1);
    outStream << Indent(indent) << "}" << std::endl;
}
StmtAST::StmtAST() : lOrExp(NULL) {}

StmtAST::StmtAST(ExpAST *lOrExp_) : lOrExp(lOrExp_) {}

StmtAST::~StmtAST()
{
    if (lOrExp)
        delete lOrExp;
}

void StmtAST ::Dump(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent) << "StmtAST { " << std::endl;
    lOrExp->Dump(outStream, indent + 1);
    outStream << Indent(indent) << "}" << std::endl;
}

ExpAST::ExpAST() : opt(OpEnum::OP_NONE), leftExp(NULL), rightExp(NULL) {}

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

void ExpAST ::Dump(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent) << "ExpAST { " << std::endl;
    if (opt == OpEnum::OP_PRI)
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
    outStream << Indent(indent) << "}" << std::endl;
}

PrimaryExpAST::PrimaryExpAST() : type(PrimEnum::PRI_NONE), exp(NULL) {}

PrimaryExpAST::PrimaryExpAST(int constVal_) : type(PrimEnum::PRI_CONST), constVal(constVal_) {}

PrimaryExpAST::PrimaryExpAST(ExpAST *exp_) : type(PrimEnum::PRI_EXP), exp(exp_) {}

PrimaryExpAST::~PrimaryExpAST() {}

void PrimaryExpAST ::Dump(std::ostream &outStream, int indent) const
{
    outStream << Indent(indent) << "PrimaryExpAST { " << std::endl;
    if (type == PrimEnum::PRI_CONST)
        outStream << Indent(indent + 1) << "Number: " << constVal << std::endl;
    if (type == PrimEnum::PRI_EXP)
        exp->Dump(outStream, indent + 1);
    outStream << Indent(indent) << "}" << std::endl;
}
