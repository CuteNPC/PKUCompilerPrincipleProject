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

  CompUnitAST      *CompUnitAST_ast_val;
  FuncDefAST       *FuncDefAST_ast_val;
  BlockAST         *BlockAST_ast_val;
  BlockItemAST     *BlockItemAST_ast_val;
  StmtAST          *StmtAST_ast_val;
  ExpAST           *ExpAST_ast_val;
  PrimaryExpAST    *PrimaryExpAST_ast_val;
  DataDeclAST      *DataDeclAST_ast_val;
  DataDefAST       *DataDefAST_ast_val;
  DataLValIdentAST *DataLValIdentAST_ast_val;
  DataInitvalAST   *DataInitvalAST_ast_val;
  FuncFParamsAST   *FuncFParamsAST_ast_val;
  FuncFParamAST    *FuncFParamAST_ast_val;
  FuncRParamsAST   *FuncRParamsAST_ast_val;
  BaseAST          *BaseAST_ast_val;
}

%token <st_val>  Y_ST_PL Y_ST_PR Y_ST_CL Y_ST_CR Y_ST_SE Y_ST_CO Y_ST_SL Y_ST_SR
%token <op_val> Y_OP_EQ Y_OP_COMP Y_OP_ADD Y_OP_MUL Y_OP_NOT_L Y_OP_AND_L Y_OP_OR_L
%token <ctrl_val> Y_CTRL_IF Y_CTRL_ELSE Y_CTRL_WHILE Y_CTRL_BREAK Y_CTRL_CONTINUE Y_CTRL_RETURN Y_CTRL_EQUAL
%token <type_val> Y_TYPE_INT Y_TYPE_VOID Y_TYPE_CONST

%token <const_int_val> Y_CONST_INT
%token <ident_val> Y_IDENT

%type <CompUnitAST_ast_val>      StartSymbol CompUnit
%type <FuncDefAST_ast_val>       FuncDef
%type <BlockAST_ast_val>         Block BlockRaw
%type <BlockItemAST_ast_val>     BlockItem
%type <StmtAST_ast_val>          Stmt
%type <ExpAST_ast_val>           Exp LOrExp LAndExp EqExp RelExp AddExp MulExp UnaryExp 
%type <ExpAST_ast_val>           ConstExp
%type <PrimaryExpAST_ast_val>    PrimaryExp
%type <DataDeclAST_ast_val>      Decl
%type <DataDeclAST_ast_val>      ConstDecl VarDecl ConstDeclRaw VarDeclRaw
%type <DataDefAST_ast_val>       ConstDef VarDef
%type <DataLValIdentAST_ast_val> ConstLValIdent VarLValIdent LVal ParaLValIdent
%type <DataInitvalAST_ast_val>   ConstInitval VarInitval ConstInitvalRaw VarInitvalRaw
%type <FuncFParamsAST_ast_val>   FuncFParams
%type <FuncFParamAST_ast_val>    FuncFParam
%type <FuncRParamsAST_ast_val>   FuncRParams

%%

StartSymbol:
  CompUnit {ast = $1;};

CompUnit:
  Decl { $$ = new CompUnitAST($1); }|
  FuncDef { $$ = new CompUnitAST($1); }|
  CompUnit Decl { ($1)->append($2); $$ = $1; }|
  CompUnit FuncDef { ($1)->append($2); $$ = $1; };

FuncDef:
  Y_TYPE_INT Y_IDENT Y_ST_PL FuncFParams Y_ST_PR Block
    { $$ = new FuncDefAST($1, mvStr($2), $4, $6); }|
  Y_TYPE_VOID Y_IDENT Y_ST_PL FuncFParams Y_ST_PR Block
    { $$ = new FuncDefAST($1, mvStr($2), $4, $6); };

Block:
  BlockRaw Y_ST_CR { $$ = $1; };

BlockRaw:
  Y_ST_CL { $$ = new BlockAST(); }|
  BlockRaw BlockItem { ($1)->append($2); $$ = $1; };

BlockItem:
  Decl { $$ = new BlockItemAST($1); }|
  Stmt { $$ = new BlockItemAST($1); };

Stmt:
  Y_ST_SE { $$ = new StmtAST(StmtEnum::STMT_EMPTY); }|
  LVal Y_CTRL_EQUAL Exp Y_ST_SE { $$ = new StmtAST($1, $3); }|
  Exp Y_ST_SE { $$ = new StmtAST(StmtEnum::STMT_EXP, $1); }|
  Y_CTRL_RETURN Exp Y_ST_SE { $$ = new StmtAST(StmtEnum::STMT_RET_INT, $2); }|
  Y_CTRL_RETURN Y_ST_SE { $$ = new StmtAST(StmtEnum::STMT_RET_VOID); }|
  Block { $$ = new StmtAST($1); }|
  Y_CTRL_IF Y_ST_PL Exp Y_ST_PR Stmt { $$ = new StmtAST(StmtEnum::STMT_IF, $3, $5); }|
  Y_CTRL_IF Y_ST_PL Exp Y_ST_PR Stmt Y_CTRL_ELSE Stmt { $$ = new StmtAST(StmtEnum::STMT_IF_ELSE, $3, $5, $7); }|
  Y_CTRL_WHILE Y_ST_PL Exp Y_ST_PR Stmt { $$ = new StmtAST(StmtEnum::STMT_WHILE, $3, $5); }|
  Y_CTRL_BREAK Y_ST_SE { $$ = new StmtAST(StmtEnum::STMT_BREAK); }|
  Y_CTRL_CONTINUE Y_ST_SE { $$ = new StmtAST(StmtEnum::STMT_CONT); } ;

ConstExp:
  Exp {$$ = $1;};

Exp:
  LOrExp { $$ = $1; };

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
  Y_OP_NOT_L UnaryExp { $$ = new ExpAST($1, $2); }|
  Y_ST_PL Exp Y_ST_PR { $$ = $2; };

PrimaryExp:
  LVal { $$ = new PrimaryExpAST($1); }|
  Y_CONST_INT { $$ = new PrimaryExpAST($1); }|
  Y_IDENT Y_ST_PL FuncRParams Y_ST_PR { $$ = new PrimaryExpAST(mvStr($1), $3); };

Decl:
  ConstDecl { $$ = $1; }|
  VarDecl { $$ = $1; };

ConstDecl:
  ConstDeclRaw Y_ST_SE { $$ = $1; }

VarDecl:
  VarDeclRaw Y_ST_SE { $$ = $1; }

ConstDeclRaw:
  Y_TYPE_CONST Y_TYPE_INT ConstDef {$$ = new DataDeclAST(DefiEnum::DEFI_CONST, $2, $3); }|
  ConstDeclRaw Y_ST_CO ConstDef {($1)->append($3); $$ = $1; };

VarDeclRaw:
  Y_TYPE_INT VarDef {$$ = new DataDeclAST(DefiEnum::DEFI_VAR, $1, $2); }|
  VarDeclRaw Y_ST_CO VarDef {($1)->append($3); $$ = $1; };

ConstDef:
  ConstLValIdent Y_CTRL_EQUAL ConstInitval {$$ = new DataDefAST(DefiEnum::DEFI_CONST, TypeEnum::TYPE_INT, $1, $3); };

VarDef:
  VarLValIdent {$$ = new DataDefAST(DefiEnum::DEFI_VAR, TypeEnum::TYPE_INT, $1); }|
  VarLValIdent Y_CTRL_EQUAL VarInitval {$$ = new DataDefAST(DefiEnum::DEFI_VAR, TypeEnum::TYPE_INT, $1, $3); };

FuncFParams:
  {$$ = new FuncFParamsAST();}|
  FuncFParam {$$ = new FuncFParamsAST($1);}|
  FuncFParams Y_ST_CO FuncFParam {($1)->append($3); $$ = $1;};

FuncFParam:
  Y_TYPE_INT ParaLValIdent {$$ = new FuncFParamAST($1, $2);};

FuncRParams:
  {$$ = new FuncRParamsAST();}|
  Exp {$$ = new FuncRParamsAST($1);}|
  FuncRParams Y_ST_CO Exp {($1)->append($3); $$ = $1;};

LVal:
  Y_IDENT {$$ = new DataLValIdentAST(DefiEnum::DEFI_LVAL, TypeEnum::TYPE_INT, mvStr($1));}|
  LVal Y_ST_SL Exp Y_ST_SR {($1)->append($3); $$ = $1;};

ConstLValIdent:
  Y_IDENT {$$ = new DataLValIdentAST(DefiEnum::DEFI_CONST, TypeEnum::TYPE_INT, mvStr($1));}|
  ConstLValIdent Y_ST_SL ConstExp Y_ST_SR {($1)->append($3); $$ = $1;};

VarLValIdent:
  Y_IDENT {$$ = new DataLValIdentAST(DefiEnum::DEFI_VAR, TypeEnum::TYPE_INT, mvStr($1));}|
  VarLValIdent Y_ST_SL ConstExp Y_ST_SR {($1)->append($3); $$ = $1;};

ParaLValIdent:
  Y_IDENT {$$ = new DataLValIdentAST(DefiEnum::DEFI_VAR, TypeEnum::TYPE_INT, mvStr($1));}|
  Y_IDENT Y_ST_SL Y_ST_SR {$$ = new DataLValIdentAST(DefiEnum::DEFI_VAR, TypeEnum::TYPE_INT, mvStr($1), true);}|
  ParaLValIdent Y_ST_SL ConstExp Y_ST_SR {($1)->append($3); $$ = $1;};

ConstInitval:
  ConstExp {$$ = new DataInitvalAST(DefiEnum::DEFI_CONST, TypeEnum::TYPE_INT, $1);}|
  Y_ST_CL Y_ST_CR {$$ = new DataInitvalAST(DefiEnum::DEFI_CONST, TypeEnum::TYPE_INT);}|
  Y_ST_CL ConstInitval Y_ST_CR {$$ = new DataInitvalAST(DefiEnum::DEFI_CONST, TypeEnum::TYPE_INT, $2);}|
  ConstInitvalRaw Y_ST_CR {$$ = $1;};

ConstInitvalRaw:
  Y_ST_CL ConstInitval {$$ = new DataInitvalAST(DefiEnum::DEFI_CONST, TypeEnum::TYPE_INT, $2);}|
  ConstInitvalRaw Y_ST_CO ConstInitval {($1)->append($3); $$ = $1;};

VarInitval:
  Exp {$$ = new DataInitvalAST(DefiEnum::DEFI_VAR, TypeEnum::TYPE_INT, $1);}|
  Y_ST_CL Y_ST_CR {$$ = new DataInitvalAST(DefiEnum::DEFI_VAR, TypeEnum::TYPE_INT);}|
  Y_ST_CL VarInitval Y_ST_CR {$$ = new DataInitvalAST(DefiEnum::DEFI_VAR, TypeEnum::TYPE_INT, $2);}|
  VarInitvalRaw Y_ST_CR {$$ = $1;};

VarInitvalRaw:
  Y_ST_CL VarInitval {$$ = new DataInitvalAST(DefiEnum::DEFI_VAR, TypeEnum::TYPE_INT, $2);}|
  VarInitvalRaw Y_ST_CO VarInitval {($1)->append($3); $$ = $1;};

%%

void yyerror(CompUnitAST* ast, const char *s) {
  cerr << "Call yyerror "<< s << endl;
}

std::string mvStr(std::string* str)
{
  string tmp(*str);
  delete str;
  return tmp;
}
