#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"
#include "koopa.h"
#include "yy.hpp"
#include "define.hpp"
#include "arg.hpp"
#include "irbuilder.hpp"
#include "kooparser.hpp"
#include "riscvbuilder.hpp"

int main(int argc, const char *argv[])
{
    Arg args(argc, argv);

    CompUnitAST *ast = yyparse(args.inputFile());

    std::cout << *ast << std::endl;

    return 0;

    IRBuilder irBuilder(ast);

    delete ast;

    std::string ir_string = irBuilder.outputString();

    if (!args.toRISCV())
    {
        args.ostream() << ir_string << std::endl;
        return 0;
    }

    KooParser kooparser(ir_string);

    koopa_raw_program_t program = kooparser.get();

    RISCVBuilder riscvbuilder(program);

    std::string riscv_string = riscvbuilder.outputString();

    args.ostream() << riscv_string << std::endl;

    return 0;
}
