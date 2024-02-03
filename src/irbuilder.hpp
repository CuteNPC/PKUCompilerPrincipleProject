#ifndef _IRBUILDER_HPP_
#define _IRBUILDER_HPP_

#include "ast.hpp"
#include "koopa.h"
#include "memory"
#include <fstream>
#include <iostream>
#include <sstream>

class IRBuilder
{
  private:
    /* data */
    CompUnitAST *ast;
    std::string funcName;
    int returnValue;
    std::string outString;

  public:
    IRBuilder();
    IRBuilder(CompUnitAST *ast_);
    void build(CompUnitAST *ast_);
    std::string outputString();
};

#endif // !_IRBUILDER_HPP_
