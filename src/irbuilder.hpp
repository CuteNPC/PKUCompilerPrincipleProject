#ifndef _IRBUILDER_HPP_
#define _IRBUILDER_HPP_

#include "ast.hpp"
#include "memory"
#include <sstream>
#include <iostream>
#include <fstream>
#include "koopa.h"

class IRBuilder
{
private:
    /* data */
    const BaseAST *ast;
    std::string funcName;
    int returnValue;
    std::string outString;

public:
    IRBuilder();
    IRBuilder(const BaseAST &ast_);
    void build(const BaseAST &ast_);
    std::string outputString();
};

#endif // !_IRBUILDER_HPP_
