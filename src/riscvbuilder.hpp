#ifndef _RISCV_BUILDER_
#define _RISCV_BUILDER_

#include "koopa.h"
#include <string>
#include "irreader.hpp"

class RISCVBuilder
{
private:
    koopa_raw_program_t program;
    std::string outString;
    IRReader irreader;

public:
    RISCVBuilder();
    RISCVBuilder(koopa_raw_program_t program);
    void build(koopa_raw_program_t program);
    std::string outputString();
};

#endif // !_RISCV_BUILDER_
