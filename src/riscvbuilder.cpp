#include "riscvbuilder.hpp"
#include <cassert>

const static char emptyMainSysYAsmString[] = "  .text\n  .globl main\nmain:\n  li a0, 0\n  ret\n";

RiscvBuilder::RiscvBuilder() : raw(NULL) {}

void RiscvBuilder::buildFrom(KoopaRaw *raw_)
{
    raw = raw_;
    return;
    return;
    return;
}

void RiscvBuilder::dump(std::ostream &outStream) const
{
    outStream << emptyMainSysYAsmString << std::endl;
}

std::ostream &operator<<(std::ostream &outStream, const RiscvBuilder &builder)
{
    builder.dump(outStream);
    return outStream;
}