#ifndef _KOOPARSER_HPP_
#define _KOOPARSER_HPP_

#include "koopa.h"
#include <string>

class KoopaParser
{
  private:
    koopa_program_t program;             /*处理程序*/
    koopa_error_code_t ret;              /*返回的错误码*/
    koopa_raw_program_builder_t builder; /*builder，最后释放，否则raw会出问题*/
    koopa_raw_program_t raw;             /*raw program*/
    bool available;

  public:
    KoopaParser();
    KoopaParser(const std::string &koopa_ir_str);
    void buildFrom(const std::string &koopa_ir_str);
    koopa_raw_program_t *getRaw();
    void checkErrorWhileBuild();
    ~KoopaParser();
};

#endif // !_KOOPARSER_HPP_