// 所有 AST 的基类
#ifndef _AST_HPP_
#define _AST_HPP_

#include <fstream>
#include <iostream>
#include <keyword.hpp>
#include <memory>

class BaseAST;

class CompUnitAST;
class FuncDefAST;
class BlockAST;
class StmtAST;
class ExpAST;
class PrimaryExpAST;

class BaseAST
{
  public:
    virtual ~BaseAST() = default;
    virtual void Dump(std::ostream &outStream = std::cout, int indent = 0) const = 0;
    friend std::ostream &operator<<(std::ostream &outStream, const BaseAST &ast);
};

class CompUnitAST : public BaseAST
{
  public:
    FuncDefAST *funcDef;
    CompUnitAST();
    CompUnitAST(FuncDefAST *funcDef_);
    virtual ~CompUnitAST();
    virtual void Dump(std::ostream &outStream = std::cout, int indent = 0) const override;
};

class FuncDefAST : public BaseAST
{
  public:
    TypeEnum funcType;
    std::string funcName;
    BlockAST *funcBody;
    FuncDefAST();
    FuncDefAST(TypeEnum funcType_, std::string funcName_, BlockAST *funcBody_);
    FuncDefAST(TypeEnum funcType_, const char *funcName_, BlockAST *funcBody_);
    virtual ~FuncDefAST();
    virtual void Dump(std::ostream &outStream = std::cout, int indent = 0) const override;
};

class BlockAST : public BaseAST
{
  public:
    StmtAST *stmt;
    BlockAST();
    BlockAST(StmtAST *stmt_);
    virtual ~BlockAST();
    virtual void Dump(std::ostream &outStream = std::cout, int indent = 0) const override;
};

class StmtAST : public BaseAST
{
  public:
    ExpAST *lOrExp;
    StmtAST();
    StmtAST(ExpAST *lOrExp_);
    virtual ~StmtAST();
    virtual void Dump(std::ostream &outStream = std::cout, int indent = 0) const override;
};

class ExpAST : public BaseAST
{
  public:
    OpEnum opt;
    ExpAST *leftExp;
    union
    {
        ExpAST *rightExp;
        PrimaryExpAST *primaryExp;
    };
    ExpAST();
    ExpAST(PrimaryExpAST *primaryExp_);
    ExpAST(OpEnum opt_, ExpAST *rightExp_);
    ExpAST(OpEnum opt_, ExpAST *leftExp_, ExpAST *rightExp_);
    virtual ~ExpAST();
    virtual void Dump(std::ostream &outStream = std::cout, int indent = 0) const override;
};

class PrimaryExpAST : public BaseAST
{
  public:
    enum PrimEnum
    {
        PRI_NONE,
        PRI_CONST,
        PRI_EXP,
    };
    PrimEnum type;
    union
    {
        int constVal;
        ExpAST *exp;
    };
    PrimaryExpAST();
    PrimaryExpAST(int constVal_);
    PrimaryExpAST(ExpAST *exp_);
    virtual ~PrimaryExpAST();
    virtual void Dump(std::ostream &outStream = std::cout, int indent = 0) const override;
};

#endif // !_AST_HPP_
