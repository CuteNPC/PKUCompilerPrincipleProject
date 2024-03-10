// 所有 AST 的基类
#ifndef _AST_HPP_
#define _AST_HPP_

#include "keyword.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

class IRBuilder;

class SymbolTable;
class SymbolEntry;

class BaseAST;

class CompUnitAST;
class FuncDefAST;
class BlockAST;
class BlockItemAST;
class StmtAST;
class ExpAST;
class PrimaryExpAST;
class DataDeclAST;
class DataDefAST;
class FuncFParamsAST;
class FuncFParamAST;
class FuncRParamsAST;
class DataLValIdentAST;
class DataInitvalAST;

class BaseAST
{
  public:
    BaseAST() = default;
    virtual ~BaseAST() = default;
    void Dump(std::ostream &outStream = std::cout, int indent = 0) const;
    virtual const char *getClassName() const = 0;
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const = 0;
    virtual void setSymbolTable(SymbolTable *symTab) = 0;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) = 0;
    friend std::ostream &operator<<(std::ostream &outStream, const BaseAST &ast);
};

class CompUnitAST : public BaseAST
{
  public:
    std::vector<DataDeclAST *> declVec;
    std::vector<FuncDefAST *> funcVec;
    CompUnitAST();
    CompUnitAST(DataDeclAST *decl_);
    CompUnitAST(FuncDefAST *func_);
    void append(DataDeclAST *decl_);
    void append(FuncDefAST *func_);
    virtual ~CompUnitAST();
    virtual const char *getClassName() const override;
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
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
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
};

class BlockAST : public BaseAST
{
  public:
    std::vector<BlockItemAST *> itemVec;
    std::vector<StmtAST *> stmtVec;
    std::vector<int> vecIndex;
    int lineIndex;
    BlockAST();
    void append(BlockItemAST *item);
    void setIndex(std::vector<int> vecIndex_, int lineIndex_);
    virtual ~BlockAST();
    virtual const char *getClassName() const override;
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
};

class BlockItemAST : public BaseAST
{
  public:
    BlockItemEnum itemEnum;
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
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
};

class StmtAST : public BaseAST
{
  public:
    StmtEnum st;
    DataLValIdentAST *lVal;
    union
    {
        ExpAST *lOrExp;
        BlockAST *block;
        void *ptr;
    };
    StmtAST *mainStmt;
    StmtAST *elseStmt;
    std::vector<int> initvalArray;
    StmtAST();
    StmtAST(StmtEnum st_);
    StmtAST(DataLValIdentAST *lVal_, ExpAST *lOrExp_);
    StmtAST(DataLValIdentAST *lVal_, std::vector<int> initvalArray_);
    StmtAST(StmtEnum st_, ExpAST *lOrExp_);
    StmtAST(BlockAST *block_);
    StmtAST(StmtEnum st_, ExpAST *lOrExp_, StmtAST *mainStmt_, StmtAST *elseStmt_ = NULL);
    virtual ~StmtAST();
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
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
    int forceCalc(SymbolTable *symTab);
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
    std::string buildIRRetString(IRBuilder *irBuilder, SymbolTable *symTab);
};

class PrimaryExpAST : public BaseAST
{
  public:
    PrimEnum type;

    int constVal;
    std::string funcName;
    union
    {
        DataLValIdentAST *lVal;
        FuncRParamsAST *paras;
        void *ptr;
    };

    PrimaryExpAST();
    PrimaryExpAST(int constVal_);
    PrimaryExpAST(DataLValIdentAST *lVal_);
    PrimaryExpAST(std::string funcName_, FuncRParamsAST *paras_);
    PrimaryExpAST(const char *funcName_, FuncRParamsAST *paras_);
    virtual ~PrimaryExpAST();
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
    int forceCalc(SymbolTable *symTab);
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
    std::string buildIRRetString(IRBuilder *irBuilder, SymbolTable *symTab);
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
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
};

class DataDefAST : public BaseAST
{
  public:
    DefiEnum defi;
    TypeEnum type;
    DataLValIdentAST *defIdent;
    DataInitvalAST *initval;
    StmtAST *stmtAfterSym;
    DataDefAST();
    DataDefAST(DefiEnum defi_, TypeEnum type_, DataLValIdentAST *defIdent_,
               DataInitvalAST *initval_ = NULL);
    virtual ~DataDefAST();
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
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
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
};

class FuncFParamAST : public BaseAST
{
  public:
    TypeEnum type;
    DataLValIdentAST *para;
    FuncFParamAST();
    FuncFParamAST(TypeEnum type_, DataLValIdentAST *para_);
    virtual ~FuncFParamAST();
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
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
    virtual void setSymbolTable(SymbolTable *symTab) override;
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
    std::string buildIRRetString(IRBuilder *irBuilder, SymbolTable *symTab);
};

class DataLValIdentAST : public BaseAST
{
  public:
    DefiEnum defi;
    TypeEnum type;
    std::string ident;
    bool emptyValStart;
    std::vector<ExpAST *> expVec;
    DataLValIdentAST();
    DataLValIdentAST(DefiEnum defi_, TypeEnum type_, std::string ident_,
                     bool emptyValStart = false);
    DataLValIdentAST(DefiEnum defi_, TypeEnum type_, const char *ident_,
                     bool emptyValStart = false);
    virtual ~DataLValIdentAST();
    void append(ExpAST *exp_);
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
    virtual void setSymbolTable(SymbolTable *symTab) override;
    std::vector<int> getArrayDim(SymbolTable *symTab = NULL);
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
    std::string buildIRRetString(IRBuilder *irBuilder, SymbolTable *symTab);
};

class DataInitvalAST : public BaseAST
{
  public:
    DefiEnum defi;
    TypeEnum type;
    ExpAST *exp;
    std::vector<DataInitvalAST *> initVec;
    DataInitvalAST();
    DataInitvalAST(DefiEnum defi_, TypeEnum type_, ExpAST *exp_);
    DataInitvalAST(DefiEnum defi_, TypeEnum type_, DataInitvalAST *initVal_ = NULL);
    virtual ~DataInitvalAST();
    void append(DataInitvalAST *initVal_);
    virtual void DumpContent(std::ostream &outStream = std::cout, int indent = 0) const override;
    virtual const char *getClassName() const override;
    virtual void setSymbolTable(SymbolTable *symTab) override;
    std::vector<int> getInitVector(std::vector<int> arrayDim, SymbolTable *symTab = NULL);
    void dfs(std::vector<int> &vec, SymbolTable *symTab = NULL);
    virtual void buildIR(IRBuilder *irBuilder, SymbolTable *symTab) override;
};

#endif // !_AST_HPP_
