#ifndef _IRREADER_HPP_
#define _IRREADER_HPP_

#include "koopa.h"
#include <string>

// 函数声明略
// ...

class IRReader
{
public:
    std::string funName;
    int returnValue;
    void Visit(const koopa_raw_slice_t &slice);
    void Visit(const koopa_raw_function_t &func);
    void Visit(const koopa_raw_basic_block_t &bb);
    void Visit(const koopa_raw_value_t &value);
    void Visit(const koopa_raw_program_t &program);
};

#endif // !_VISITOR_HPP_
