#include "arg.hpp"
#include "ast.hpp"
#include "define.hpp"
#include "irbuilder.hpp"
#include "koopa.h"
#include "koopaparser.hpp"
#include "riscvbuilder.hpp"
#include "symtab.hpp"
#include "yy.hpp"
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#define MYDEBUG

int main(int argc, const char *argv[])
{
    Arg args(argc, argv);

    CompUnitAST *ast = yyparse(args.inputFile());

    SymbolTable *symTab = new SymbolTable();
    symTab->buildFrom(ast);

    IRBuilder *irBuilder = new IRBuilder();
    irBuilder->buildFrom(ast, symTab);

    if (args.toKoopa())
    {
        args.ostream() << *irBuilder << std::endl;
        return 0;
    }

    std::stringstream koopaIRStream;
    koopaIRStream << *irBuilder << std::endl;
    std::string koopaIRString = koopaIRStream.str();

#ifdef MYDEBUG
    std::ofstream koopaOut("temp/koopa.txt");
    koopaOut << koopaIRString << std::endl;
    koopaOut.close();
#endif

    KoopaParser koopaParser(koopaIRString);

    RiscvBuilder *riscvBuilder = new RiscvBuilder();
    riscvBuilder->buildFrom(koopaParser.getRaw());

    args.ostream() << *riscvBuilder << std::endl;

    return 0;
}
