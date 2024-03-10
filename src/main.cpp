#include "arg.hpp"
#include "ast.hpp"
#include "define.hpp"
#include "irbuilder.hpp"
#include "koopa.h"
#include "symtab.hpp"
#include "yy.hpp"
#include <iostream>
#include <memory>
#include <string>

int main(int argc, const char *argv[])
{
    Arg args(argc, argv);

    CompUnitAST *ast = yyparse(args.inputFile());

    args.ostream() << *ast << std::endl;

    SymbolTable *symTab = new SymbolTable();

    symTab->buildFrom(ast);

    std::ofstream symInfoFile("temp/sym.txt");

    symInfoFile << *symTab << std::endl;

    std::ofstream astInfoFile("temp/res2.txt");

    astInfoFile << *ast << std::endl;

    IRBuilder *irBuilder = new IRBuilder();

    irBuilder->buildFrom(ast, symTab);

    return 0;
}
