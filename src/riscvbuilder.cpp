#include "riscvbuilder.hpp"

const static char emptyMainSysYAsmString[] = "  .text\n  .globl %s\n%s:\n  li a0, %d\n  ret\n";

RISCVBuilder::RISCVBuilder()
{
}

RISCVBuilder::RISCVBuilder(koopa_raw_program_t program_)
{
    build(program_);
}
void RISCVBuilder::build(koopa_raw_program_t program_)
{
    program = program_;

    irreader.Visit(program);
}
std::string RISCVBuilder::outputString()
{
    char buffer[0x100];
    sprintf(buffer, emptyMainSysYAsmString, irreader.funName.c_str(),
            irreader.funName.c_str(), irreader.returnValue);
    outString = buffer;
    return outString;
}