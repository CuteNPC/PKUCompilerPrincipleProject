#include "yy.hpp"
#include <iostream>

extern FILE *yyin;
extern int yyparse(std::unique_ptr<BaseAST> &ast);
extern std::unique_ptr<BaseAST> yyparse(FILE *file);

std::unique_ptr<BaseAST> yyparse(FILE *file)
{
    std::unique_ptr<BaseAST> ast;
    yyin = file;
    int ret = yyparse(ast);
    if (ret)
        std::cerr << "yyparse error" << std::endl;
    return ast;
}
