#ifndef _RISCV_BUILDER_
#define _RISCV_BUILDER_

#include "koopa.h"
#include <iostream>
#include <string>

class RiscvBuilder
{

  private:
    koopa_raw_program_t *rawProgram;

  public:
    RiscvBuilder();
    void buildFrom(koopa_raw_program_t *rawProgram_);
    void dump(std::ostream &outStream = std::cout) const;
    friend std::ostream &operator<<(std::ostream &outStream, const RiscvBuilder &builder);
    void visitRawProg(const koopa_raw_program_t &rawProg);
    void visitGloData(const koopa_raw_value_t &gloData);
    void visitFunc(const koopa_raw_function_t &func);
    void visitBlock(const koopa_raw_basic_block_t &block);
    void visitStmt(const koopa_raw_value_t &stmt);
};

#endif // !_RISCV_BUILDER_
