#ifndef _KETWORD_HPP_
#define _KETWORD_HPP_

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
};

enum TypeEnum
{
    TYPE_NONE,
    TYPE_VOID,
    TYPE_INT,
    TYPE_CONST,
};

#endif // !_LEX_HPP_
