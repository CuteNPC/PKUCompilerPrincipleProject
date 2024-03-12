#include "irbuilder.hpp"
#include "keyword.hpp"
#include <functional>

// const static char emptyMainKoopaIRString[] = "fun @%s(): i32 {\n%%entry:\n  ret %d\n}\n";

/* IRBlock */

IRBlock::IRBlock() : blockName(), deadBlock(), stmtVec() {}

IRBlock::IRBlock(std::string blockName_, bool deadBlock_, std::vector<std::string> stmtVec_)
    : blockName(blockName_), deadBlock(deadBlock_), stmtVec(stmtVec_)
{
}

void IRBlock::dump(std::ostream &outStream) const
{
    outStream << "  " << blockName << ':' << std::endl;
    for (const std::string &elem : stmtVec)
        outStream << "    " << elem << std::endl;
    outStream << std::endl;
}

std::ostream &operator<<(std::ostream &outStream, const IRBlock &block)
{
    block.dump(outStream);
    return outStream;
}

/* IRFunction */

IRFunction::IRFunction() : funcName(), inputType(), outputType(), blockVec() {}

IRFunction::IRFunction(std::string funcName_, std::string inputType_, std::string outputType_,
                       std::vector<IRBlock *> blockVec_)
    : funcName(funcName_), inputType(inputType_), outputType(outputType_), blockVec(blockVec_)
{
}

void IRFunction::dump(std::ostream &outStream) const
{
    outStream << "fun @" << funcName << '(' << inputType << ')' << outputType << std::endl;
    outStream << "{" << std::endl;
    outStream << std::endl;
    for (IRBlock *block : blockVec)
        if ((!block->deadBlock) && (block->stmtVec.size() != 0))
            outStream << *block << std::endl;
    outStream << '}' << std::endl;
}

std::ostream &operator<<(std::ostream &outStream, const IRFunction &func)
{
    func.dump(outStream);
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

void IRBuilder::dump(std::ostream &outStream) const
{
    for (auto p = libFuncDecl; (*p)[0]; p++)
        outStream << "decl @" << (*p)[0] << '(' << (*p)[1] << ')' << (*p)[2] << std::endl;
    outStream << std::endl;
    for (const std::string &data : dataVec)
        outStream << data << std::endl;
    outStream << std::endl;
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

IRBlock *IRBuilder::getNewBlock(bool nextIsDeadBlock)
{
    return new IRBlock(getNextIdent(), nextIsDeadBlock);
}

void IRBuilder::setCurrentBlock(IRBlock *block)
{
    if (currentBlock)
        delete currentBlock;
    currentBlock = block;
}

void IRBuilder::pushCurrentBlock()
{
    static const bool pushDeadBlock = true;
    if ((!currentBlock->deadBlock) || pushDeadBlock)
        blockVec.push_back(currentBlock);
    else
        delete currentBlock;
    currentBlock = NULL;
}

void IRBuilder::pushAndGetBlock(bool nextIsDeadBlock)
{
    pushCurrentBlock();
    IRBlock *block = getNewBlock(nextIsDeadBlock);
    setCurrentBlock(block);
}

void IRBuilder::pushStmt(std::string stmt) { currentBlock->stmtVec.push_back(stmt); }

void IRBuilder::endFunc()
{
    pushCurrentBlock();
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

void IRBuilder::connectWhile(std::string cond, IRBlock *entryBlock, std::string testName,
                             IRBlock *testBlock, std::string mainName, IRBlock *mainBlock,
                             std::string endName)
{
    std::string entryJumpStmt = std::string("jump ") + testName;
    entryBlock->stmtVec.push_back(entryJumpStmt);

    std::string testJumpStmt =
        std::string("br ") + cond + std::string(", ") + mainName + std::string(", ") + endName;
    testBlock->stmtVec.push_back(testJumpStmt);

    std::string mainJumpStmt = std::string("jump ") + testName;
    mainBlock->stmtVec.push_back(mainJumpStmt);
}
std::string IRBuilder::getIRType(const std::vector<int> &arrayDim_)
{
    std::string typeName("i32");
    for (auto iter = arrayDim_.rbegin(); iter != arrayDim_.rend(); iter++)
    {
        int dim = *iter;
        if (dim != -1)
            typeName = std::string("[") + typeName + ", " + std::to_string(dim) + "]";
        else
            typeName = std::string("*") + typeName;
    }
    return typeName;
}

std::string IRBuilder::aggregate1DtoNDString(const std::vector<int> &initvalVec,
                                             const std::vector<int> &arrayDim)
{
    if (initvalVec.size() == 0)
        return std::string("zeroinit");

    std::vector<int>::const_iterator valIter = initvalVec.begin(), dimIter = arrayDim.begin(),
                                     dimIterEnd = arrayDim.end();

    std::function<std::string(std::vector<int>::const_iterator)> factorial =
        [&factorial, &valIter, &dimIterEnd](std::vector<int>::const_iterator dimIter) -> std::string
    {
        if (dimIter == dimIterEnd)
            return std::to_string(*(valIter++));
        std::string retString = "{";
        for (int i = 0; i < *dimIter; i++)
            retString += std::string(i ? ", " : "") + factorial(dimIter + 1);
        retString += "}";
        return retString;
    };
    return factorial(dimIter);
}

std::ostream &operator<<(std::ostream &outStream, const IRBuilder &build)
{
    build.dump(outStream);
    return outStream;
}

/* END */
