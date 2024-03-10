#include "irbuilder.hpp"

// const static char emptyMainKoopaIRString[] = "fun @%s(): i32 {\n%%entry:\n  ret %d\n}\n";

/* IRStmt */

IRStmt::IRStmt() : hello(0) {}

/* IRBlock */

IRBlock::IRBlock() : blockName(), deadBlock(), stmtVec() {}

IRBlock::IRBlock(std::string blockName_, bool deadBlock_, std::vector<std::string> stmtVec_)
    : blockName(blockName_), deadBlock(deadBlock_), stmtVec(stmtVec_)
{
}

void IRBlock::Dump(std::ostream &outStream) const
{
    outStream << blockName << ':' << std::endl;
    for (const std::string &elem : stmtVec)
        outStream << "  " << elem << std::endl;
    outStream << std::endl;
}

std::ostream &operator<<(std::ostream &outStream, const IRBlock &block)
{
    block.Dump(outStream);
    return outStream;
}

/* IRFunction */

IRFunction::IRFunction() : funcName(), inputType(), outputType(), blockVec() {}

IRFunction::IRFunction(std::string funcName_, std::string inputType_, std::string outputType_,
                       std::vector<IRBlock *> blockVec_)
    : funcName(funcName_), inputType(inputType_), outputType(outputType_), blockVec(blockVec_)
{
}

void IRFunction::Dump(std::ostream &outStream) const
{
    outStream << "fun @" << funcName << '(' << inputType << ')' << outputType << " {" << std::endl;
    outStream << std::endl;
    for (IRBlock *block : blockVec)
        if ((!block->deadBlock) && (block->stmtVec.size() != 0))
            outStream << *block << std::endl;
    outStream << '}' << std::endl;
}

std::ostream &operator<<(std::ostream &outStream, const IRFunction &func)
{
    func.Dump(outStream);
    return outStream;
}

/* IRBuilder */

IRBuilder::IRBuilder()
    : currentTempCounter(0), currentBlock(NULL), funcName(), blockVec(), dataVec(), funcVec()
{
}

void IRBuilder::buildFrom(CompUnitAST *ast, SymbolTable *symTab)
{
    symTab->resetCursor();
    ast->buildIR(this, symTab);
    symTab->resetCursor();
}

void IRBuilder::Dump(std::ostream &outStream) const
{
    // for (IRGloData *data : dataVec)
    // outStream << *data << std::endl;
    for (IRFunction *func : funcVec)
        outStream << *func << std::endl;
}

void IRBuilder::startFunc(std::string funcName_, std::string inputType_, std::string outputType_)
{
    currentTempCounter = 0;
    funcName = funcName_;
    inputType = inputType_;
    outputType = outputType_;
    blockVec.clear();

    currentBlock = new IRBlock(getNextIdent());
}

void IRBuilder::pushBlock(bool nextIsDeadBlock)
{
    static const bool pushDeadBlock = true;
    if ((!currentBlock->deadBlock) || pushDeadBlock)
        blockVec.push_back(currentBlock);
    else
        delete currentBlock;

    currentBlock = new IRBlock(getNextIdent(), nextIsDeadBlock);
}

void IRBuilder::pushStmt(std::string stmt) { currentBlock->stmtVec.push_back(stmt); }

void IRBuilder::endFunc()
{
    pushBlock();
    delete currentBlock;
    IRFunction *irFunction = new IRFunction(funcName, inputType, outputType, blockVec);
    funcVec.push_back(irFunction);

    currentBlock = NULL;

    currentTempCounter = 0;
    funcName.clear();
    inputType.clear();
    outputType.clear();
    blockVec.clear();
}

std::string IRBuilder::getNextIdent()
{
    std::string res("%");
    res += std::to_string(currentTempCounter);
    currentTempCounter++;
    return res;
}

void IRBuilder::connectIf(std::string cond, IRBlock *entryBlock, std::string thenName,
                          IRBlock *thenBlock, std::string endName)
{
    std::string condJumpStmt =
        std::string("br ") + cond + std::string(", ") + thenName + std::string(", ") + endName;
    entryBlock->stmtVec.push_back(condJumpStmt);

    std::string endJumpStmt = std::string("jump ") + endName;
    thenBlock->stmtVec.push_back(endJumpStmt);
}

void IRBuilder::connectIfElse(std::string cond, IRBlock *entryBlock, std::string thenName,
                              IRBlock *thenBlock, std::string elseName, IRBlock *elseBlock,
                              std::string endName)
{
    std::string condJumpStmt =
        std::string("br ") + cond + std::string(", ") + thenName + std::string(", ") + elseName;
    entryBlock->stmtVec.push_back(condJumpStmt);

    std::string endJumpStmt = std::string("jump ") + endName;
    thenBlock->stmtVec.push_back(endJumpStmt);
    elseBlock->stmtVec.push_back(endJumpStmt);
}

std::ostream &operator<<(std::ostream &outStream, const IRBuilder &build)
{
    build.Dump(outStream);
    return outStream;
}

/* END */
