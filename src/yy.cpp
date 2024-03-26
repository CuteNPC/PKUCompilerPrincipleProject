#include "yy.hpp"
#include <iostream>

extern FILE *yyin;
extern int yyparse(CompUnitAST *&ast);
extern CompUnitAST *yyparse(FILE *file);

CompUnitAST *yyparse(FILE *file)
{
    CompUnitAST *ast;
    yyin = file;
    int ret = yyparse(ast);
    if (ret)
    {
        std::cerr << "yyparse error" << std::endl;
        exit(-1);
    }
    return ast;
}
