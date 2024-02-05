// 所有 AST 的基类
#ifndef _AST_HPP_
#define _AST_HPP_

#include <fstream>
#include <iostream>
#include <keyword.hpp>
#include <memory>
#include <vector>

class BaseAST;

class CompUnitAST;
class FuncDefAST;
class BlockAST;
class BlockItemAST;
class StmtAST;
class ExpAST;
class PrimaryExpAST;
class DataDefAST;
class DataDeclAST;
class FuncFParamsAST;
class FuncFParamAST;
class FuncRParamsAST;

class BaseAST
{
  public:
    BaseAST() = default;
    virtual ~BaseAST() = default;
    void Dump(std::ostream &outStream = std::cout, int indent = 0) const;
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const = 0;
    virtual const char *getClassName() const = 0;
    friend std::ostream &operator<<(std::ostream &outStream, const BaseAST &ast);
};

class CompUnitAST : public BaseAST
{
    enum DorF
    {
        DORF_NONE,
        DORF_DECL,
        DORF_FUNC,
    };
    struct DeclOrFunc
    {
        DorF eNum;
        union
        {
            void *ptr;
            DataDeclAST *decl;
            FuncDefAST *func;
        };
    };

  public:
    std::vector<DeclOrFunc> vec;
    CompUnitAST();
    CompUnitAST(DataDeclAST *decl_);
    CompUnitAST(FuncDefAST *func_);
    void append(DataDeclAST *decl_);
    void append(FuncDefAST *func_);
    virtual ~CompUnitAST();
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
};

class FuncDefAST : public BaseAST
{
  public:
    TypeEnum funcType;
    std::string funcName;
    FuncFParamsAST *paras;
    BlockAST *funcBody;
    FuncDefAST();
    FuncDefAST(TypeEnum funcType_, std::string funcName_, FuncFParamsAST *paras_,
               BlockAST *funcBody_);
    FuncDefAST(TypeEnum funcType_, const char *funcName_, FuncFParamsAST *paras_,
               BlockAST *funcBody_);
    virtual ~FuncDefAST();
    virtual const char *getClassName() const override;
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
};

class BlockAST : public BaseAST
{
  public:
    std::vector<BlockItemAST *> itemVec;
    BlockAST();
    void append(BlockItemAST *item);
    virtual ~BlockAST();
    virtual const char *getClassName() const override;
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
};

class BlockItemAST : public BaseAST
{
  public:
    enum BlockItemEnum
    {
        BLOCK_NONE,
        BLOCK_DECL,
        BLOCK_STMT,
    } itemEnum;
    union
    {
        DataDeclAST *decl;
        StmtAST *stmt;
        void *ptr;
    };
    BlockItemAST();
    BlockItemAST(DataDeclAST *decl_);
    BlockItemAST(StmtAST *stmt_);
    virtual ~BlockItemAST();
    virtual const char *getClassName() const override;
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
};

enum StmtEnum
{
    STMT_NONE,
    STMT_EMPTY,
    STMT_ASSIGN,
    STMT_EXP,
    STMT_RET_INT,
    STMT_RET_VOID,
    STMT_BLOCK,
    STMT_IF,
    STMT_IF_ELSE,
    STMT_WHILE,
    STMT_BREAK,
    STMT_CONT,
};

class StmtAST : public BaseAST
{
  public:
    StmtEnum st;
    std::string ident;
    union
    {
        ExpAST *lOrExp;
        BlockAST *block;
        void *ptr;
    };
    StmtAST *mainStmt;
    StmtAST *elseStmt;
    StmtAST();
    StmtAST(StmtEnum st_);
    StmtAST(std::string ident_, ExpAST *lOrExp_);
    StmtAST(const char *ident_, ExpAST *lOrExp_);
    StmtAST(StmtEnum st_, ExpAST *lOrExp_);
    StmtAST(BlockAST *block_);
    StmtAST(StmtEnum st_, ExpAST *lOrExp_, StmtAST *mainStmt_, StmtAST *elseStmt_ = NULL);
    virtual ~StmtAST();
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
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
        void *ptr;
    };
    ExpAST();
    ExpAST(PrimaryExpAST *primaryExp_);
    ExpAST(OpEnum opt_, ExpAST *rightExp_);
    ExpAST(OpEnum opt_, ExpAST *leftExp_, ExpAST *rightExp_);
    virtual ~ExpAST();
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
};

class PrimaryExpAST : public BaseAST
{
  public:
    enum PrimEnum
    {
        PRI_NONE,
        PRI_CONST,
        PRI_LVAL,
        PRI_CALL,
    };
    PrimEnum type;

    int constVal;
    std::string ident;
    FuncRParamsAST *paras;

    PrimaryExpAST();
    PrimaryExpAST(int constVal_);
    PrimaryExpAST(std::string lValName_);
    PrimaryExpAST(const char *lValName_);
    PrimaryExpAST(std::string funcName_, FuncRParamsAST *paras_);
    PrimaryExpAST(const char *funcName_, FuncRParamsAST *paras_);
    virtual ~PrimaryExpAST();
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
};

enum DefiEnum
{
    DEFI_NONE,
    DEFI_VAR,
    DEFI_CONST,
};

class DataDeclAST : public BaseAST
{
  public:
    DefiEnum defi;
    TypeEnum type;
    std::vector<DataDefAST *> defVec;
    DataDeclAST();
    DataDeclAST(DefiEnum defi_, TypeEnum type_, DataDefAST *def_ = NULL);
    virtual ~DataDeclAST();
    void append(DataDefAST *def_);
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
};

class DataDefAST : public BaseAST
{
  public:
    DefiEnum defi;
    TypeEnum type;
    std::string ident;
    ExpAST *exp;
    DataDefAST();
    DataDefAST(DefiEnum defi_, std::string ident_, ExpAST *exp_ = NULL);
    DataDefAST(DefiEnum defi_, const char *ident_, ExpAST *exp_ = NULL);
    virtual ~DataDefAST();
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
};

class FuncFParamsAST : public BaseAST
{
  public:
    std::vector<FuncFParamAST *> paraVec;
    FuncFParamsAST();
    FuncFParamsAST(FuncFParamAST *para_);
    virtual ~FuncFParamsAST();
    void append(FuncFParamAST *para_);
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
};

class FuncFParamAST : public BaseAST
{
  public:
    TypeEnum type;
    std::string ident;
    FuncFParamAST();
    FuncFParamAST(TypeEnum type_, std::string ident_);
    FuncFParamAST(TypeEnum type_, const char *ident_);
    virtual ~FuncFParamAST();
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
};

class FuncRParamsAST : public BaseAST
{
  public:
    std::vector<ExpAST *> expVec;
    FuncRParamsAST();
    FuncRParamsAST(ExpAST *exp_);
    virtual ~FuncRParamsAST();
    void append(ExpAST *exp_);
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
};
/*
class DefIdent : public BaseAST
{
  public:
    DefiEnum defi;
    TypeEnum type;
    std::string ident;
    bool emptyValStart;
    vector<ExpAST *> expVec;
    DefIdent();
    DefIdent(std::string ident_, bool emptyValStart = false);
    DefIdent(const char *ident_, bool emptyValStart = false);
    virtual ~DefIdent();
    void append(ExpAST *exp_);
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
};

class DefInitval : public BaseAST
{
  public:
    DefiEnum defi;
    TypeEnum type;
    ExpAST *exp;
    vector<DefInitval *> initList;
    DefInitval();
    DefInitval(ExpAST *exp);
    DefInitval(DefInitval *initVal_);
    virtual ~DefInitval();
    void append(DefInitval *initVal_);
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
};
*/
#endif // !_AST_HPP_
