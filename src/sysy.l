%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>
#include "ast.hpp"
#include "keyword.hpp"
#include "sysy.tab.hpp"

%}

WhiteSpace    [ \t\n\r]*
LineComment   "//".*
BlockComment  \/\*([^*]|(\*+([^*/])))*\*+\/

Identifier    [a-zA-Z_][a-zA-Z0-9_]*

Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

%%

{WhiteSpace}    { }
{LineComment}   { }
{BlockComment}  { }

"("             { yylval.st_val   = StructEnum::ST_PL               ; return Y_ST_PL         ; }
")"             { yylval.st_val   = StructEnum::ST_PR               ; return Y_ST_PR         ; }
"["             { yylval.st_val   = StructEnum::ST_SL               ; return Y_ST_SL         ; }
"]"             { yylval.st_val   = StructEnum::ST_SR               ; return Y_ST_SR         ; }
"{"             { yylval.st_val   = StructEnum::ST_CL               ; return Y_ST_CL         ; }
"}"             { yylval.st_val   = StructEnum::ST_CR               ; return Y_ST_CR         ; }
";"             { yylval.st_val   = StructEnum::ST_SE               ; return Y_ST_SE         ; }
","             { yylval.st_val   = StructEnum::ST_CO               ; return Y_ST_CO         ; }

"+"             { yylval.op_val   = OpEnum::OP_ADD                  ; return Y_OP_ADD        ; }
"-"             { yylval.op_val   = OpEnum::OP_SUB                  ; return Y_OP_ADD        ; }
"*"             { yylval.op_val   = OpEnum::OP_MUL                  ; return Y_OP_MUL        ; }
"/"             { yylval.op_val   = OpEnum::OP_DIV                  ; return Y_OP_MUL        ; }
"%"             { yylval.op_val   = OpEnum::OP_MOD                  ; return Y_OP_MUL        ; }
"<"             { yylval.op_val   = OpEnum::OP_L                    ; return Y_OP_COMP       ; }
">"             { yylval.op_val   = OpEnum::OP_G                    ; return Y_OP_COMP       ; }
"!"             { yylval.op_val   = OpEnum::OP_NOT_L                ; return Y_OP_NOT_L      ; }
"=="            { yylval.op_val   = OpEnum::OP_E                    ; return Y_OP_EQ         ; }
"!="            { yylval.op_val   = OpEnum::OP_NE                   ; return Y_OP_EQ         ; }
"<="            { yylval.op_val   = OpEnum::OP_LE                   ; return Y_OP_COMP       ; }
">="            { yylval.op_val   = OpEnum::OP_GE                   ; return Y_OP_COMP       ; }
"&&"            { yylval.op_val   = OpEnum::OP_AND_L                ; return Y_OP_AND_L      ; }
"||"            { yylval.op_val   = OpEnum::OP_OR_L                 ; return Y_OP_OR_L       ; }

"="             { yylval.ctrl_val = CtrlEnum::CTRL_EQUAL            ; return Y_CTRL_EQUAL    ; }
"if"            { yylval.ctrl_val = CtrlEnum::CTRL_IF               ; return Y_CTRL_IF       ; }
"else"          { yylval.ctrl_val = CtrlEnum::CTRL_ELSE             ; return Y_CTRL_ELSE     ; }
"while"         { yylval.ctrl_val = CtrlEnum::CTRL_WHILE            ; return Y_CTRL_WHILE    ; }
"break"         { yylval.ctrl_val = CtrlEnum::CTRL_BREAK            ; return Y_CTRL_BREAK    ; }
"continue"      { yylval.ctrl_val = CtrlEnum::CTRL_CONTINUE         ; return Y_CTRL_CONTINUE ; }
"return"        { yylval.ctrl_val = CtrlEnum::CTRL_RETURN           ; return Y_CTRL_RETURN   ; }

"int"           { yylval.type_val = TypeEnum::TYPE_INT              ; return Y_TYPE_INT      ; }
"void"          { yylval.type_val = TypeEnum::TYPE_VOID             ; return Y_TYPE_VOID     ; }
"const"         { yylval.type_val = TypeEnum::TYPE_CONST            ; return Y_TYPE_CONST    ; }

{Identifier}    { yylval.ident_val = new std::string(yytext)             ; return Y_IDENT         ; }

{Decimal}       { yylval.const_int_val = strtol(yytext, nullptr, 0) ; return Y_CONST_INT     ; }
{Octal}         { yylval.const_int_val = strtol(yytext, nullptr, 0) ; return Y_CONST_INT     ; }
{Hexadecimal}   { yylval.const_int_val = strtol(yytext, nullptr, 0) ; return Y_CONST_INT     ; }

%%
