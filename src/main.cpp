#include "arg.hpp"
#include "ast.hpp"
#include "define.hpp"
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

    SymbolTable *symTab = new SymbolTable();

    args.ostream() << *ast << std::endl;

    ofstream symInfoFile("temp/sym.txt");

    ast->setSymbolTable(symTab);

    symInfoFile << *symTab << std::endl;

    ofstream astInfoFile("temp/res2.txt");

    astInfoFile << *ast << std::endl;

    return 0;
}
