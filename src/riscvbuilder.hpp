#ifndef _RISCV_BUILDER_
#define _RISCV_BUILDER_

#include "koopa.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

class RiscvBuilder
{
  public:
    koopa_raw_program_t *rawProgram;

    int funcCommandCount;
    std::vector<int> funcAllocArray;
    int mem4Byte;
    int funcCount;

    int currentCommandIndex;
    int currentAllocedCount;
    std::map<std::string, int> varTable;

    std::vector<std::string> instVec;

    RiscvBuilder();
    void buildFrom(koopa_raw_program_t *rawProgram_);

    void visitRawProg(const koopa_raw_program_t &rawProg);
    void visitGloData(const koopa_raw_value_t &gloData);

    void countFunc(const koopa_raw_function_t &func);
    void visitFunc(const koopa_raw_function_t &func);

    void countBlock(const koopa_raw_basic_block_t &block);
    void visitBlock(const koopa_raw_basic_block_t &block);

    void countStmt(const koopa_raw_value_t &stmt);
    void visitStmt(const koopa_raw_value_t &stmt);

    bool validOffset(int offset);
    std::vector<std::string> loadValue(const koopa_raw_value_t &value, const char *distReg);
    std::vector<std::string> storeValue(const koopa_raw_value_t &value, const char *distReg);

    void pushAInst(std::string ainst);
    void pushPInst(std::string pinst);
    void pushLabel(std::string label);
    void pushCment(std::string cment);

    void pushAInst(const char *ainst);
    void pushPInst(const char *pinst);
    void pushLabel(const char *label);
    void pushCment(const char *cment);

    void pushAInst(const std::vector<std::string> &ainstVec);
    void pushEmpty();

    void pushVarIndex(std::string name);
    void pushVarIndex(const char *name);
    void pushVarIndex();

    int matchVarIndex(std::string name);
    int matchVarIndex(const char *name);

    int calcArrayTypeSize(koopa_raw_type_t ty);
    std::vector<int> aggregateNDto1DVector(const koopa_raw_value_t value);
    void dump(std::ostream &outStream = std::cout) const;
    friend std::ostream &operator<<(std::ostream &outStream, const RiscvBuilder &builder);
};

#endif // !_RISCV_BUILDER_
