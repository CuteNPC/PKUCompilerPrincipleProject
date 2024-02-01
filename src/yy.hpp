#ifndef _YY_HPP_
#define _YY_HPP_

#include <memory>
#include "ast.hpp"

extern std::unique_ptr<BaseAST> yyparse(FILE* file);

#endif // !_YY_HPP_
