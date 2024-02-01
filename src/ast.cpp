#include "ast.hpp"

std::ostream &operator<<(std::ostream &outStream, const BaseAST &ast)
{
    ast.Dump(outStream);
    return outStream;
}

void CompUnitAST::Dump(std::ostream &outStream = std::cout) const
{
    outStream << "CompUnitAST { ";
    func_def->Dump(outStream);
    outStream << " }";
}

void FuncDefAST ::Dump(std::ostream &outStream = std::cout) const
{
    outStream << "FuncDefAST { ";
    func_type->Dump(outStream);
    outStream << ", " << ident << ", ";
    block->Dump(outStream);
    outStream << " }";
}

void FuncTypeAST ::Dump(std::ostream &outStream = std::cout) const
{
    outStream << "FuncTypeAST { ";
    outStream << "int";
    outStream << " }";
}

void BlockAST ::Dump(std::ostream &outStream = std::cout) const
{
    outStream << "BlockAST { ";
    stmt->Dump(outStream);
    outStream << " }";
}

void StmtAST ::Dump(std::ostream &outStream = std::cout) const
{
    outStream << "StmtAST { ";
    outStream << val;
    outStream << " }";
}
