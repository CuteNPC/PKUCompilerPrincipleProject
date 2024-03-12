#include "riscvbuilder.hpp"
#include <cassert>

const static char emptyMainSysYAsmString[] = "  .text\n  .globl main\nmain:\n  li a0, 0\n  ret\n";

RiscvBuilder::RiscvBuilder() : rawProgram(NULL) {}

void RiscvBuilder::buildFrom(koopa_raw_program_t *rawProgram_)
{
    rawProgram = rawProgram_;
    visitRawProg(*rawProgram);
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

// 访问 raw program
void RiscvBuilder::visitRawProg(const koopa_raw_program_t &rawProg)
{
    // 执行一些其他的必要操作
    // ...

    assert(rawProg.values.kind == KOOPA_RSIK_VALUE);
    assert(rawProg.funcs.kind == KOOPA_RSIK_FUNCTION);
    for (size_t i = 0; i < rawProg.values.len; i++)
        visitGloData((koopa_raw_value_t)(rawProg.values.buffer[i]));
    for (size_t i = 0; i < rawProg.funcs.len; i++)
        visitFunc((koopa_raw_function_t)(rawProg.funcs.buffer[i]));
}

void RiscvBuilder::visitGloData(const koopa_raw_value_t &gloData) { return; }

void RiscvBuilder::visitFunc(const koopa_raw_function_t &func)
{
    assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
    for (size_t i = 0; i < func->bbs.len; i++)
        visitBlock((koopa_raw_basic_block_t)(func->bbs.buffer[i]));
}

void RiscvBuilder::visitBlock(const koopa_raw_basic_block_t &block)
{
    assert(block->insts.kind == KOOPA_RSIK_VALUE);
    for (size_t i = 0; i < block->insts.len; i++)
        visitStmt((koopa_raw_value_t)(block->insts.buffer[i]));
}

void RiscvBuilder::visitStmt(const koopa_raw_value_t &stmt)
{
    // 根据指令类型判断后续需要如何访问
    const auto &kind = stmt->kind;
    switch (kind.tag)
    {
    case KOOPA_RVT_INTEGER:
    {
        /// Integer constant.
        std::cout << "KOOPA_RVT_INTEGER" << std::endl;
    }
    break;
    case KOOPA_RVT_ZERO_INIT:
    {
        /// Zero initializer.
        std::cout << "KOOPA_RVT_ZERO_INIT" << std::endl;
    }
    break;
    case KOOPA_RVT_UNDEF:
    {
        /// Undefined value.
        std::cout << "KOOPA_RVT_UNDEF" << std::endl;
    }
    break;
    case KOOPA_RVT_AGGREGATE:
    {
        /// Aggregate constant.
        std::cout << "KOOPA_RVT_AGGREGATE" << std::endl;
    }
    break;
    case KOOPA_RVT_FUNC_ARG_REF:
    {
        /// Function argument reference.
        std::cout << "KOOPA_RVT_FUNC_ARG_REF" << std::endl;
    }
    break;
    case KOOPA_RVT_BLOCK_ARG_REF:
    {
        /// Basic block argument reference.
        std::cout << "KOOPA_RVT_BLOCK_ARG_REF" << std::endl;
    }
    break;
    case KOOPA_RVT_ALLOC:
    {
        /// Local memory allocation.
        std::cout << "KOOPA_RVT_ALLOC" << std::endl;
    }
    break;
    case KOOPA_RVT_GLOBAL_ALLOC:
    {
        /// Global memory allocation.
        std::cout << "KOOPA_RVT_GLOBAL_ALLOC" << std::endl;
    }
    break;
    case KOOPA_RVT_LOAD:
    {
        /// Memory load.
        std::cout << "KOOPA_RVT_LOAD" << std::endl;
    }
    break;
    case KOOPA_RVT_STORE:
    {
        /// Memory store.
        std::cout << "KOOPA_RVT_STORE" << std::endl;
    }
    break;
    case KOOPA_RVT_GET_PTR:
    {
        /// Pointer calculation.
        std::cout << "KOOPA_RVT_GET_PTR" << std::endl;
    }
    break;
    case KOOPA_RVT_GET_ELEM_PTR:
    {
        /// Element pointer calculation.
        std::cout << "KOOPA_RVT_GET_ELEM_PTR" << std::endl;
    }
    break;
    case KOOPA_RVT_BINARY:
    {
        /// Binary operation.
        std::cout << "KOOPA_RVT_BINARY" << std::endl;
    }
    break;
    case KOOPA_RVT_BRANCH:
    {
        /// Conditional branch.
        std::cout << "KOOPA_RVT_BRANCH" << std::endl;
    }
    break;
    case KOOPA_RVT_JUMP:
    {
        /// Unconditional jump.
        std::cout << "KOOPA_RVT_JUMP" << std::endl;
    }
    break;
    case KOOPA_RVT_CALL:
    {
        /// Function call.
        std::cout << "KOOPA_RVT_CALL" << std::endl;
    }
    break;
    case KOOPA_RVT_RETURN:
    {
        /// Function return.
        std::cout << "KOOPA_RVT_RETURN" << std::endl;
    }
    break;
    default:
        // 其他类型暂时遇不到
        assert(false);
    }
}
