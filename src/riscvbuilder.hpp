#ifndef _RISCV_BUILDER_
#define _RISCV_BUILDER_

#include "koopa.h"
#include <iostream>
#include <string>

typedef koopa_raw_program_t KoopaRaw;

class RiscvBuilder
{

  private:
    KoopaRaw *raw;

  public:
    RiscvBuilder();
    void buildFrom(KoopaRaw *program);
    void dump(std::ostream &outStream = std::cout) const;
    friend std::ostream &operator<<(std::ostream &outStream, const RiscvBuilder &builder);
};

#endif // !_RISCV_BUILDER_
