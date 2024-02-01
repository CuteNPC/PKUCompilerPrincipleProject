#ifndef _KOOPARSER_HPP_
#define _KOOPARSER_HPP_

#include "koopa.h"
#include <string>
class KooParser
{
private:
    std::string inString;
    koopa_program_t program;
    koopa_error_code_t ret;
    koopa_raw_program_builder_t builder;
    koopa_raw_program_t raw;

public:
    KooParser();
    KooParser(const std::string &koopa_ir_str);
    void build(const std::string &koopa_ir_str);
    koopa_raw_program_t get();
    ~KooParser();
};

#endif // !_KOOPARSER_HPP_