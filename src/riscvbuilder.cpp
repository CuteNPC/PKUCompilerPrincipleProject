#include "riscvbuilder.hpp"
#include <cassert>
#include <vector>

// const static char emptyMainSysYAsmString[] = "  .text\n  .globl main\nmain:\n  li a0, 0\n ret\n";

const static char *binaryOPInst[][2] = {{"sub", "snez t0, t0"},
                                        {"sub", "seqz t0, t0"},
                                        {"sgt", NULL},
                                        {"slt", NULL},
                                        {"slt", "xori t0, t0, 1"},
                                        {"sgt", "xori t0, t0, 1"},
                                        {"add", NULL},
                                        {"sub", NULL},
                                        {"mul", NULL},
                                        {"div", NULL},
                                        {"rem", NULL},
                                        {"and", NULL},
                                        {"or", NULL},
                                        {"xor", NULL},
                                        {"sll", NULL},
                                        {"srl", NULL},
                                        {"sra", NULL}};

RiscvBuilder::RiscvBuilder()
    : rawProgram(NULL), funcCommandCount(0), funcAllocArray(), mem4Byte(0), funcCount(0),
      currentCommandIndex(0), varTable(), instVec()
{
}

void RiscvBuilder::buildFrom(koopa_raw_program_t *rawProgram_)
{
    rawProgram = rawProgram_;
    visitRawProg(*rawProgram);
}

void RiscvBuilder::dump(std::ostream &outStream) const
{
    for (const std::string &inst : instVec)
        outStream << inst << std::endl;
}

std::ostream &operator<<(std::ostream &outStream, const RiscvBuilder &builder)
{
    builder.dump(outStream);
    return outStream;
}

void RiscvBuilder::visitRawProg(const koopa_raw_program_t &rawProg)
{
    // 执行一些其他的必要操作
    // ...

    assert(rawProg.values.kind == KOOPA_RSIK_VALUE);
    assert(rawProg.funcs.kind == KOOPA_RSIK_FUNCTION);
    for (size_t i = 0; i < rawProg.values.len; i++)
        visitGloData((koopa_raw_value_t)(rawProg.values.buffer[i]));
    pushPInst("text");
    for (funcCount = 0; funcCount < rawProg.funcs.len; funcCount++)
    {
        countFunc((koopa_raw_function_t)(rawProg.funcs.buffer[funcCount]));
        visitFunc((koopa_raw_function_t)(rawProg.funcs.buffer[funcCount]));
    }
}

void RiscvBuilder::visitGloData(const koopa_raw_value_t &gloData) { return; }

void RiscvBuilder::countFunc(const koopa_raw_function_t &func)
{
    static const int moreSpace = 16;

    funcCommandCount = 0;
    funcAllocArray.clear();

    assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
    for (size_t i = 0; i < func->bbs.len; i++)
        countBlock((koopa_raw_basic_block_t)(func->bbs.buffer[i]));

    mem4Byte = (funcCommandCount + moreSpace + 3) & (-3);
    for (int elem : funcAllocArray)
        mem4Byte += elem;
}

void RiscvBuilder::visitFunc(const koopa_raw_function_t &func)
{
    currentCommandIndex = 0;
    currentAllocedCount = 0;
    varTable.clear();

    /* TODO 这种判断对吗 */
    if (func->bbs.len == 0)
        return;

    /* 进入函数，分配栈空间 */
    if (std::string(func->name + 1) == std::string("main"))
    {
        pushPInst("globl main");
        pushEmpty();
        pushLabel(func->name + 1);
    }
    else
        pushLabel(std::string("FUNC_") + (func->name + 1));
    pushCment("prologue");
    pushAInst("addi sp, sp, " + std::to_string(-mem4Byte * 4));
    pushEmpty();

    assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
    for (size_t i = 0; i < func->bbs.len; i++)
        visitBlock((koopa_raw_basic_block_t)(func->bbs.buffer[i]));

    pushEmpty();
}

void RiscvBuilder::countBlock(const koopa_raw_basic_block_t &block)
{
    assert(block->insts.kind == KOOPA_RSIK_VALUE);
    for (size_t i = 0; i < block->insts.len; i++)
        countStmt((koopa_raw_value_t)(block->insts.buffer[i]));
}

void RiscvBuilder::visitBlock(const koopa_raw_basic_block_t &block)
{
    assert(block->insts.kind == KOOPA_RSIK_VALUE);
    pushLabel("BLOCK_" + std::to_string(funcCount) + "_" + (block->name + 1));
    for (size_t i = 0; i < block->insts.len; i++)
        visitStmt((koopa_raw_value_t)(block->insts.buffer[i]));
    pushEmpty();
}

void RiscvBuilder::countStmt(const koopa_raw_value_t &stmt)
{
    funcCommandCount++;
    if (stmt->kind.tag != KOOPA_RVT_ALLOC)
    {
        funcAllocArray.push_back(0);
        return;
    }
    koopa_raw_type_t ty = stmt->ty->data.pointer.base;
    int arraySize = 1;
    bool whileFlag = true;
    while (whileFlag)
    {
        switch (ty->tag)
        {
        case KOOPA_RTT_INT32:
            whileFlag = false;
            break;
        case KOOPA_RTT_UNIT:
            assert(false);
            break;
        case KOOPA_RTT_ARRAY:
            arraySize *= ty->data.array.len;
            ty = ty->data.array.base;
            break;
        case KOOPA_RTT_POINTER:
            whileFlag = false;
            break;
        case KOOPA_RTT_FUNCTION:
            assert(false);
            break;
        default:
            assert(false);
        }
    }
    funcAllocArray.push_back(arraySize);
}

void RiscvBuilder::visitStmt(const koopa_raw_value_t &stmt)
{
    // 根据指令类型判断后续需要如何访问
    pushVarIndex(stmt->name);

    switch (stmt->kind.tag)
    {
    case KOOPA_RVT_INTEGER:
    {
        /// Integer constant.
        pushCment("KOOPA_RVT_INTEGER");
    }
    break;
    case KOOPA_RVT_ZERO_INIT:
    {
        /// Zero initializer.
        pushCment("KOOPA_RVT_ZERO_INIT");
    }
    break;
    case KOOPA_RVT_UNDEF:
    {
        /// Undefined value.
        pushCment("KOOPA_RVT_UNDEF");
    }
    break;
    case KOOPA_RVT_AGGREGATE:
    {
        /// Aggregate constant.
        pushCment("KOOPA_RVT_AGGREGATE");
    }
    break;
    case KOOPA_RVT_FUNC_ARG_REF:
    {
        /// Function argument reference.
        pushCment("KOOPA_RVT_FUNC_ARG_REF");
    }
    break;
    case KOOPA_RVT_BLOCK_ARG_REF:
    {
        /// Basic block argument reference.
        pushCment("KOOPA_RVT_BLOCK_ARG_REF");
    }
    break;
    case KOOPA_RVT_ALLOC:
    {
        /// Local memory allocation.
        pushCment("KOOPA_RVT_ALLOC");

        pushAInst("addi t0, sp, " + std::to_string((currentAllocedCount + funcCommandCount) * 4));
        pushAInst("sw t0, " + std::to_string(currentCommandIndex * 4) + "(sp)");
        currentAllocedCount += funcAllocArray[currentCommandIndex];
    }
    break;
    case KOOPA_RVT_GLOBAL_ALLOC:
    {
        /// Global memory allocation.
        pushCment("KOOPA_RVT_GLOBAL_ALLOC");
    }
    break;
    case KOOPA_RVT_LOAD:
    {
        /// Memory load.
        pushCment("KOOPA_RVT_LOAD");

        const koopa_raw_load_t &load = stmt->kind.data.load;
        pushAInst(loadValue(load.src, "t0"));
        pushAInst("lw t1, 0(t0)");
        pushAInst(storeValue(stmt, "t1"));
    }
    break;
    case KOOPA_RVT_STORE:
    {
        /// Memory store.
        pushCment("KOOPA_RVT_STORE");

        const koopa_raw_store_t &store = stmt->kind.data.store;
        pushAInst(loadValue(store.value, "t0"));
        pushAInst(loadValue(store.dest, "t1"));
        pushAInst("sw t0, 0(t1)");
    }
    break;
    case KOOPA_RVT_GET_PTR:
    {
        /// Pointer calculation.
        pushCment("KOOPA_RVT_GET_PTR");
    }
    break;
    case KOOPA_RVT_GET_ELEM_PTR:
    {
        /// Element pointer calculation.
        pushCment("KOOPA_RVT_GET_ELEM_PTR");
    }
    break;
    case KOOPA_RVT_BINARY:
    {
        /// Binary operation.
        pushCment("KOOPA_RVT_BINARY");

        const koopa_raw_binary_t &binary = stmt->kind.data.binary;
        pushAInst(loadValue(binary.lhs, "t1"));
        pushAInst(loadValue(binary.rhs, "t2"));
        pushAInst(std::string(binaryOPInst[binary.op][0]) + " t0, t1, t2");
        if (binaryOPInst[binary.op][1])
            pushAInst(binaryOPInst[binary.op][1]);
        pushAInst(storeValue(stmt, "t0"));
    }
    break;
    case KOOPA_RVT_BRANCH:
    {
        /// Conditional branch.
        pushCment("KOOPA_RVT_BRANCH");

        const koopa_raw_branch_t &branch = stmt->kind.data.branch;
        pushAInst(loadValue(branch.cond, "t0"));
        std::string str = " t0, BLOCK_" + std::to_string(funcCount) + "_";
        pushAInst("bnez" + str + (branch.true_bb->name + 1));
        pushAInst("beqz" + str + (branch.false_bb->name + 1));
    }
    break;
    case KOOPA_RVT_JUMP:
    {
        /// Unconditional jump.
        pushCment("KOOPA_RVT_JUMP");

        const koopa_raw_jump_t &jump = stmt->kind.data.jump;
        pushAInst("j BLOCK_" + std::to_string(funcCount) + "_" + (jump.target->name + 1));
    }
    break;
    case KOOPA_RVT_CALL:
    {
        /// Function call.
        pushCment("KOOPA_RVT_CALL");
    }
    break;
    case KOOPA_RVT_RETURN:
    {
        /// Function return.
        pushCment("KOOPA_RVT_RETURN");

        const koopa_raw_return_t &ret = stmt->kind.data.ret;
        if (ret.value)
            pushAInst(loadValue(ret.value, "a0"));
        pushAInst("addi sp, sp, " + std::to_string(mem4Byte * 4));
        pushAInst("ret");
    }
    break;
    default:
        // 其他类型暂时遇不到
        assert(false);
    }

    currentCommandIndex++;
}

std::string RiscvBuilder::loadValue(const koopa_raw_value_t &value, const char *distReg)
{
    std::string dist(distReg);
    if (value->name)
        return ("lw " + dist + ", " + std::to_string(matchVarIndex(value->name) * 4) + "(sp)");
    else if (value->kind.tag == KOOPA_RVT_INTEGER)
        return ("li " + dist + ", " + std::to_string(value->kind.data.integer.value));
    assert(false);
    return std::string("error");
}

std::string RiscvBuilder::storeValue(const koopa_raw_value_t &value, const char *distReg)
{
    std::string dist(distReg);
    if (value->name)
        return ("sw " + dist + ", " + std::to_string(matchVarIndex(value->name) * 4) + "(sp)");
    assert(false);
    return std::string("error");
}

void RiscvBuilder::pushAInst(std::string ainst) { instVec.push_back("    " + ainst); }

void RiscvBuilder::pushPInst(std::string pinst) { instVec.push_back("    ." + pinst); }

void RiscvBuilder::pushLabel(std::string label) { instVec.push_back(label + ":"); }

void RiscvBuilder::pushCment(std::string cment) { instVec.push_back("    # " + cment); }

void RiscvBuilder::pushAInst(const char *ainst) { pushAInst(std::string(ainst)); }

void RiscvBuilder::pushPInst(const char *pinst) { pushPInst(std::string(pinst)); }

void RiscvBuilder::pushLabel(const char *label) { pushLabel(std::string(label)); }

void RiscvBuilder::pushCment(const char *cment) { pushCment(std::string(cment)); }

void RiscvBuilder::pushEmpty() { instVec.push_back(std::string()); }

void RiscvBuilder::pushVarIndex(std::string name) { varTable.push_back(name); }

void RiscvBuilder::pushVarIndex(const char *name) { pushVarIndex(std::string(name ? name : "")); }

void RiscvBuilder::pushVarIndex() { pushVarIndex(std::string()); }

int RiscvBuilder::matchVarIndex(std::string name)
{
    int len = varTable.size();
    for (int i = 0; i < len; i++)
        if (varTable[i] == name)
            return i;
    assert(false);
    return -1;
}

int RiscvBuilder::matchVarIndex(const char *name) { return matchVarIndex(std::string(name)); }

/* END */