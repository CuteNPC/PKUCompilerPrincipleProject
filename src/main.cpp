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

#define MYDEBUGx

#ifdef MYDEBUG

std::ofstream astInfoFile1("temp/ast1.txt");
std::ofstream symInfoFile("temp/sym.txt");
std::ofstream astInfoFile2("temp/ast2.txt");

#else

std::ofstream astInfoFile1("/dev/null");
std::ofstream symInfoFile("/dev/null");
std::ofstream astInfoFile2("/dev/null");

#endif

int main(int argc, const char *argv[])
{
    Arg args(argc, argv);

    CompUnitAST *ast = yyparse(args.inputFile());

    astInfoFile1 << *ast << std::endl;

    SymbolTable *symTab = new SymbolTable();

    symTab->buildFrom(ast);

    symInfoFile << *symTab << std::endl;

    astInfoFile2 << *ast << std::endl;

    IRBuilder *irBuilder = new IRBuilder();

    irBuilder->buildFrom(ast, symTab);

    args.ostream() << *irBuilder << std::endl;

    return 0;
}
