#include "arg.hpp"
#include "ast.hpp"
#include "define.hpp"
#include "koopa.h"
#include "yy.hpp"
#include "symtab.hpp"
#include <iostream>
#include <memory>
#include <string>

int main(int argc, const char *argv[])
{
    Arg args(argc, argv);

    CompUnitAST *ast = yyparse(args.inputFile());

    SymbolTable *symTab = new SymbolTable();

    args.ostream() << *ast << std::endl;
    
    ast->setSymbolTable(symTab);


    return 0;
}
