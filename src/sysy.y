%code requires {
  #include <memory>
  #include <string>
  #include <iostream>
  #include "ast.hpp"
  #include "keyword.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"
#include "keyword.hpp"

int yylex();
void yyerror(CompUnitAST* ast, const char *s);
std::string mvStr(std::string* str);

using namespace std;

%}

%parse-param { CompUnitAST* &ast }

%union {
  StructEnum st_val;
  OpEnum op_val;
  CtrlEnum ctrl_val;
  TypeEnum type_val;

  int const_int_val;
  std::string *ident_val;

  CompUnitAST   *CompUnitAST_ast_val;
  FuncDefAST    *FuncDefAST_ast_val;
  BlockAST      *BlockAST_ast_val;
  StmtAST       *StmtAST_ast_val;
  ExpAST        *ExpAST_ast_val;
  PrimaryExpAST *PrimaryExpAST_ast_val;
  BaseAST       *BaseAST_ast_val;
}

%token <st_val>  Y_ST_PL Y_ST_PR Y_ST_SL Y_ST_SR Y_ST_CL Y_ST_CR Y_ST_SE Y_ST_EQ
%token <op_val> Y_OP_EQ Y_OP_COMP Y_OP_ADD Y_OP_MUL Y_OP_NOT_L Y_OP_AND_L Y_OP_OR_L
%token <ctrl_val> Y_CTRL_IF Y_CTRL_ELSE Y_CTRL_WHILE Y_CTRL_BREAK Y_CTRL_CONTINUE Y_CTRL_RETURN
%token <type_val> Y_TYPE_INT Y_TYPE_VOID Y_TYPE_CONST

%token <const_int_val> Y_CONST_INT
%token <ident_val> Y_IDENT

%type <CompUnitAST_ast_val>   CompUnit
%type <FuncDefAST_ast_val>    FuncDef
%type <BlockAST_ast_val>      Block
%type <StmtAST_ast_val>       Stmt
%type <ExpAST_ast_val>        LOrExp LAndExp EqExp RelExp AddExp MulExp UnaryExp
%type <PrimaryExpAST_ast_val> PrimaryExp
 
%%

CompUnit:
  FuncDef { ast = new CompUnitAST($1); };

FuncDef:
  Y_TYPE_INT Y_IDENT Y_ST_PL Y_ST_PR Block
    { $$ = new FuncDefAST($1, mvStr($2), $5); };

Block:
  Y_ST_CL Stmt Y_ST_CR { $$ = new BlockAST($2); };

Stmt:
  Y_CTRL_RETURN LOrExp Y_ST_SE { $$ = new StmtAST($2); };

LOrExp:
  LAndExp { $$ = $1; }|
  LOrExp Y_OP_OR_L LAndExp{ $$ = new ExpAST($2, $1, $3); };

LAndExp:
  EqExp { $$ = $1; }|
  LAndExp Y_OP_AND_L EqExp { $$ = new ExpAST($2, $1, $3); };

EqExp:
  RelExp { $$ = $1; }|
  EqExp Y_OP_EQ RelExp { $$ = new ExpAST($2, $1, $3); };

RelExp:
  AddExp { $$ = $1; }|
  RelExp Y_OP_COMP AddExp { $$ = new ExpAST($2, $1, $3); };

AddExp:
  MulExp { $$ = $1; }|
  AddExp Y_OP_ADD MulExp { $$ = new ExpAST($2, $1, $3); };

MulExp:
  UnaryExp { $$ = $1; }|
  MulExp Y_OP_MUL UnaryExp { $$ = new ExpAST($2, $1, $3); };

UnaryExp: 
  PrimaryExp { $$ = new ExpAST($1); }|
  Y_OP_ADD UnaryExp { $$ = new ExpAST($1, $2); }|
  Y_OP_NOT_L UnaryExp { $$ = new ExpAST($1, $2); };

PrimaryExp:
  Y_ST_PL LOrExp Y_ST_PR { $$ = new PrimaryExpAST($2); }|
  Y_CONST_INT { $$ = new PrimaryExpAST($1); };

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(CompUnitAST* ast, const char *s) {
  cerr << "Call yyerror" << endl;
}

std::string mvStr(std::string* str)
{
  string tmp = *str;
  delete str;
  return tmp;
}


/*

当前的语法

lex 任务：
解析以下内容：
1.整数
2.运算符
+ - * / %
3.内置关键字
  3.1 各种类型 int void
  3.2 内置标识符 if else while return break continue
4.单字符（三种左括号，有括号，分号）

CompUnit    ::= FuncDef;
FuncDef     ::= FuncType IDENT "(" ")" Block;
FuncType    ::= "int";
Block       ::= "{" Stmt "}";
Stmt        ::= "return" LOrExp ";";

LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
LAndExp     ::= EqExp | LAndExp "&&" EqExp;
EqExp       ::= RelExp | EqExp EqOp RelExp;
RelExp      ::= AddExp | RelExp CompOp AddExp;
EqOp        ::= "==" | "!=";
CompOp      ::= "<" | ">" | "<=" | ">=";

AddExp      ::= MulExp | AddExp AddOp MulExp;
AddOp       ::= "+" | "-";
MulExp      ::= UnaryExp | MulExp MulOp UnaryExp;
MulOp       ::= "*" | "/" | "%";

UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
PrimaryExp  ::= "(" LOrExp ")" | Number;
UnaryOp     ::= "+" | "-" | "!";

Number      ::= INT_CONST;




example：

Stmt: RETURN Number ';' {
    auto ast = new StmtAST();
    ast->val = $2;
    $$ = move(ast);
  }
  ;


*/
