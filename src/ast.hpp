// 所有 AST 的基类
#ifndef _AST_HPP_
#define _AST_HPP_

#include <memory>
#include <iostream>
#include <fstream>

class BaseAST
{
public:
    virtual ~BaseAST() = default;
    virtual void Dump(std::ostream &outStream) const = 0;
    friend std::ostream &operator<<(std::ostream &outStream, const BaseAST &ast);
};

class CompUnitAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_def;
    void Dump(std::ostream &outStream) const override;
};

class FuncDefAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;
    void Dump(std::ostream &outStream) const override;
};

class FuncTypeAST : public BaseAST
{
public:
    void Dump(std::ostream &outStream) const override;
};

class BlockAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> stmt;
    void Dump(std::ostream &outStream) const override;
};

class StmtAST : public BaseAST
{
public:
    int val;
    void Dump(std::ostream &outStream) const override;
};

// ...

#endif // !_AST_HPP_
