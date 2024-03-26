#include "riscvbuilder.hpp"
#include <cassert>
#include <functional>
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

const static char *argReg[] = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};

static const int argsCountInReg = 8;

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
    pushPInst("data");
    pushEmpty();
    for (size_t i = 0; i < rawProg.values.len; i++)
        visitGloData((koopa_raw_value_t)(rawProg.values.buffer[i]));
    pushPInst("text");
    pushEmpty();
    for (funcCount = 0; funcCount < rawProg.funcs.len; funcCount++)
    {
        countFunc((koopa_raw_function_t)(rawProg.funcs.buffer[funcCount]));
        visitFunc((koopa_raw_function_t)(rawProg.funcs.buffer[funcCount]));
    }
}

void RiscvBuilder::visitGloData(const koopa_raw_value_t &gloData)
{
    pushPInst(std::string("globl ") + (gloData->name + 1));
    pushLabel((gloData->name + 1));

    assert(gloData->kind.tag == KOOPA_RVT_GLOBAL_ALLOC);
    const koopa_raw_global_alloc_t &global_alloc = gloData->kind.data.global_alloc;
    const koopa_raw_value_kind_t &kind = global_alloc.init->kind;

    if (kind.tag == KOOPA_RVT_INTEGER)
        pushPInst("word " + std::to_string(kind.data.integer.value));
    else if (kind.tag == KOOPA_RVT_AGGREGATE)
    {
        std::vector<int> vec = aggregateNDto1DVector(global_alloc.init);
        for (int elem : vec)
            pushPInst("word " + std::to_string(elem));
    }
    else if (kind.tag == KOOPA_RVT_ZERO_INIT)
        pushPInst("zero " + std::to_string(calcArrayTypeSize(gloData->ty->data.pointer.base) * 4));

    pushEmpty();
    return;
}

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

    /* TODO 函数声明，直接返回，但是这种判断对吗 */
    if (func->bbs.len == 0)
        return;

    /* 设置函数头 */
    std::string funcName(func->name + 1);
    if (funcName == "main")
    {
        pushPInst("globl main");
        pushEmpty();
    }
    pushLabel(funcName);

    /* 进入函数，分配栈空间 */
    pushCment("prologue");
    pushAInst("sw ra, -4(sp)");
    pushAInst("li t0, " + std::to_string(-mem4Byte * 4));
    pushAInst("add sp, sp, t0");
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
    int arraySize = calcArrayTypeSize(stmt->ty->data.pointer.base);
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
        assert(false);
    }
    break;
    case KOOPA_RVT_ZERO_INIT:
    {
        /// Zero initializer.
        pushCment("KOOPA_RVT_ZERO_INIT");
        assert(false);
    }
    break;
    case KOOPA_RVT_UNDEF:
    {
        /// Undefined value.
        pushCment("KOOPA_RVT_UNDEF");
        assert(false);
    }
    break;
    case KOOPA_RVT_AGGREGATE:
    {
        /// Aggregate constant.
        pushCment("KOOPA_RVT_AGGREGATE");
        assert(false);
    }
    break;
    case KOOPA_RVT_FUNC_ARG_REF:
    {
        /// Function argument reference.
        pushCment("KOOPA_RVT_FUNC_ARG_REF");
        assert(false);
    }
    break;
    case KOOPA_RVT_BLOCK_ARG_REF:
    {
        /// Basic block argument reference.
        pushCment("KOOPA_RVT_BLOCK_ARG_REF");
        assert(false);
    }
    break;
    case KOOPA_RVT_ALLOC:
    {
        /// Local memory allocation.
        pushCment("KOOPA_RVT_ALLOC");
        pushAInst("li t1, " + std::to_string((currentAllocedCount + funcCommandCount) * 4));
        pushAInst("add t0, sp, t1");
        pushAInst("sw t0, " + std::to_string(currentCommandIndex * 4) + "(sp)");
        currentAllocedCount += funcAllocArray[currentCommandIndex];
    }
    break;
    case KOOPA_RVT_GLOBAL_ALLOC:
    {
        /// Global memory allocation.
        pushCment("KOOPA_RVT_GLOBAL_ALLOC");
        assert(false);
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
        if (store.value->kind.tag != KOOPA_RVT_AGGREGATE)
        {
            pushAInst(loadValue(store.value, "t0"));
            pushAInst(loadValue(store.dest, "t1"));
            pushAInst("sw t0, 0(t1)");
        }
        else
        {
            pushAInst(loadValue(store.dest, "t1"));
            std::vector<int> vec = aggregateNDto1DVector(store.value);
            int length = vec.size();
            for (int i = 0; i < length; i++)
            {
                pushAInst("li t0, " + std::to_string(vec[i]));
                pushAInst("sw t0, 0(t1)");
                pushAInst("addi t1, t1, 4");
            }
        }
    }
    break;
    case KOOPA_RVT_GET_PTR:
    {
        /// Pointer calculation.
        pushCment("KOOPA_RVT_GET_PTR");

        const koopa_raw_get_ptr_t &get_ptr = stmt->kind.data.get_ptr;
        pushAInst(loadValue(get_ptr.src, "t0"));
        pushAInst(loadValue(get_ptr.index, "t1"));
        int size = calcArrayTypeSize(get_ptr.src->ty->data.pointer.base) * 4;
        pushAInst("li t2, " + std::to_string(size));
        pushAInst("mul t3, t1, t2");
        pushAInst("add t0, t0, t3");
        pushAInst(storeValue(stmt, "t0"));
    }
    break;
    case KOOPA_RVT_GET_ELEM_PTR:
    {
        /// Element pointer calculation.
        pushCment("KOOPA_RVT_GET_ELEM_PTR");

        const koopa_raw_get_elem_ptr_t &get_elem_ptr = stmt->kind.data.get_elem_ptr;
        pushAInst(loadValue(get_elem_ptr.src, "t0"));
        pushAInst(loadValue(get_elem_ptr.index, "t1"));
        int size = calcArrayTypeSize(get_elem_ptr.src->ty->data.pointer.base->data.array.base) * 4;
        pushAInst("li t2, " + std::to_string(size));
        pushAInst("mul t3, t1, t2");
        pushAInst("add t0, t0, t3");
        pushAInst(storeValue(stmt, "t0"));
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

        const koopa_raw_call_t &call = stmt->kind.data.call;

        /*计算*/
        int argLength = (int)(call.args.len);
        int regParamCount = std::min(argsCountInReg, argLength);

        /*寄存器上分配参数*/
        for (uint32_t i = 0; i < regParamCount; i++)
            pushAInst(loadValue((koopa_raw_value_t)(call.args.buffer[i]), argReg[i]));

        /*栈上分配参数*/
        for (int i = argsCountInReg; i < argLength; i++)
        {
            pushAInst(loadValue((koopa_raw_value_t)(call.args.buffer[i]), "t0"));
            pushAInst("sw t0, " + std::to_string((i - argLength) * 4) + "(sp)");
        }

        /*分配栈上参数空间*/
        if (argLength > argsCountInReg)
            pushAInst("addi sp, sp, " + std::to_string((argsCountInReg - argLength) * 4));

        /*调用函数*/
        pushAInst("call " + std::string(call.callee->name + 1));

        /*释放栈上参数空间*/
        if (argLength > argsCountInReg)
            pushAInst("addi sp, sp, " + std::to_string((argLength - argsCountInReg) * 4));

        /*获得返回值*/
        if (stmt->name)
            pushAInst(storeValue(stmt, "a0"));
    }
    break;
    case KOOPA_RVT_RETURN:
    {
        /// Function return.
        pushCment("KOOPA_RVT_RETURN");

        const koopa_raw_return_t &ret = stmt->kind.data.ret;
        if (ret.value)
            pushAInst(loadValue(ret.value, "a0"));
        pushAInst("li t0, " + std::to_string(mem4Byte * 4));
        pushAInst("add sp, sp, t0");
        pushAInst("lw ra, -4(sp)");
        pushAInst("ret");
    }
    break;
    default:
        // 其他类型暂时遇不到
        assert(false);
    }

    currentCommandIndex++;
}

std::vector<std::string> RiscvBuilder::loadValue(const koopa_raw_value_t &value,
                                                 const char *distReg)
{
    std::string dist(distReg);
    std::vector<std::string> vec;
    if (value->kind.tag == KOOPA_RVT_FUNC_ARG_REF)
    {
        int index = (int)(value->kind.data.func_arg_ref.index);
        if (index < argsCountInReg)
            vec.push_back("mv " + dist + ", " + argReg[index]);
        else
        {
            vec.push_back("li t5, " + std::to_string((mem4Byte + index - argsCountInReg) * 4));
            vec.push_back(std::string("add t5, t5, sp"));
            vec.push_back("lw " + dist + ", 0(t5)");
        }
    }
    else if (value->name)
    {
        int index = matchVarIndex(value->name);
        if (index != -1)
            vec.push_back("lw " + dist + ", " + std::to_string(index * 4) + "(sp)");
        else
            vec.push_back("la " + dist + ", " + (value->name + 1));
    }
    else if (value->kind.tag == KOOPA_RVT_INTEGER)
        vec.push_back("li " + dist + ", " + std::to_string(value->kind.data.integer.value));
    else
    {
        assert(false);
        vec.push_back(std::string("error"));
    }
    return vec;
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

void RiscvBuilder::pushAInst(const std::vector<std::string> &ainstVec)
{
    for (const std::string &ainst : ainstVec)
        instVec.push_back("    " + ainst);
}

void RiscvBuilder::pushEmpty() { instVec.push_back(std::string()); }

void RiscvBuilder::pushVarIndex(std::string name)
{
    static int count = 0;
    int value = varTable.size();
    if (varTable.find(name) != varTable.end())
        varTable["+-*/" + name + std::to_string(count++)] = value;
    else
        varTable[name] = value;
}

void RiscvBuilder::pushVarIndex(const char *name) { pushVarIndex(std::string(name ? name : "")); }

void RiscvBuilder::pushVarIndex() { pushVarIndex(std::string()); }

int RiscvBuilder::matchVarIndex(std::string name)
{
    if (varTable.find(name) == varTable.end())
        return -1;
    return varTable[name];
}

int RiscvBuilder::matchVarIndex(const char *name) { return matchVarIndex(std::string(name)); }

int RiscvBuilder::calcArrayTypeSize(koopa_raw_type_t ty)
{
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
    return arraySize;
}

std::vector<int> RiscvBuilder::aggregateNDto1DVector(const koopa_raw_value_t value)
{
    std::vector<int> result;

    std::function<void(const koopa_raw_value_t)> factorial =
        [&result, &factorial](const koopa_raw_value_t value) -> void
    {
        if (value->kind.tag == KOOPA_RVT_AGGREGATE)
        {
            const koopa_raw_slice_t &slice = value->kind.data.aggregate.elems;
            for (int i = 0; i < slice.len; i++)
                factorial((koopa_raw_value_t)(slice.buffer[i]));
        }
        else if (value->kind.tag == KOOPA_RVT_INTEGER)
            result.push_back(value->kind.data.integer.value);
        else
            assert(false);
    };

    factorial(value);

    return result;
}

/* END */