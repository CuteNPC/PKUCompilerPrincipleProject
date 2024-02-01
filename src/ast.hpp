// 所有 AST 的基类
#ifndef _AST_HPP_
#define _AST_HPP_

#include <memory>
#include <iostream>

class BaseAST
{
public:
    virtual ~BaseAST() = default;
    virtual void Dump() const = 0;
};

class CompUnitAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_def;
    void Dump() const override;
};

class FuncDefAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    void Dump() const override;
};

class FuncTypeAST : public BaseAST
{
public:
    std::string type;
    void Dump() const override;
};

class BlockAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> stmt;
    void Dump() const override;
};

class StmtAST : public BaseAST
{
public:
    int val;
    void Dump() const override;
};

// ...

#endif // !_AST_HPP_