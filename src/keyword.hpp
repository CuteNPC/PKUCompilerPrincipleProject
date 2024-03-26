#ifndef _KETWORD_HPP_
#define _KETWORD_HPP_

extern const char *defiName[];
extern const char *typeName[];
extern const char *optName[];
extern const char *libFuncDecl[][3];
extern const char *riscvInst[];

enum OpEnum
{
    OP_NONE,

    OP_PRI,

    OP_E,
    OP_NE,
    OP_L,
    OP_LE,
    OP_G,
    OP_GE,

    OP_POS,
    OP_NEG,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    OP_NOT_B,
    OP_AND_B,
    OP_OR_B,
    OP_XOR_B,

    OP_NOT_L,
    OP_AND_L,
    OP_OR_L,
};

enum StructEnum
{
    ST_NONE,

    ST_PL = '(',
    ST_PR = ')',

    ST_SL = '[',
    ST_SR = ']',

    ST_CL = '{',
    ST_CR = '}',

    ST_SE = ';',
    ST_EQ = '=',

    ST_CO = ',',
    ST_PE = '.',

};

enum CtrlEnum
{
    CTRL_NONE,
    CTRL_IF,
    CTRL_ELSE,
    CTRL_WHILE,
    CTRL_BREAK,
    CTRL_CONTINUE,
    CTRL_RETURN,
    CTRL_EQUAL,
};

enum TypeEnum
{
    TYPE_NONE,
    TYPE_VOID,
    TYPE_INT,
    TYPE_CONST,
};

enum BlockItemEnum
{
    BLOCK_NONE,
    BLOCK_DECL,
    BLOCK_STMT,
};

enum StmtEnum
{
    STMT_NONE,
    STMT_EMPTY,
    STMT_ASSIGN,
    STMT_ASSIGN_ARRAY,
    STMT_EXP,
    STMT_RET_INT,
    STMT_RET_VOID,
    STMT_BLOCK,
    STMT_IF,
    STMT_IF_ELSE,
    STMT_WHILE,
    STMT_BREAK,
    STMT_CONT,
};

enum PrimEnum
{
    PRI_NONE,
    PRI_CONST,
    PRI_LVAL,
    PRI_CALL,
};

enum DefiEnum
{
    DEFI_NONE,
    DEFI_VAR,
    DEFI_CONST,
    DEFI_LVAL,
};

#endif // !_LEX_HPP_
