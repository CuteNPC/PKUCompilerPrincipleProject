#include <stddef.h>

const char *defiName[] = {"NONE", "Var", "Const", "LVal"};
const char *typeName[] = {"NONE", "void", "int", "const"};
const char *optName[] = {

    "OP_NONE",

    "OP_PRI",

    "OP_E",     "OP_NE",    "OP_L",    "OP_LE",    "OP_G",   "OP_GE",

    "OP_POS",   "OP_NEG",   "OP_ADD",  "OP_SUB",   "OP_MUL", "OP_DIV", "OP_MOD",

    "OP_NOT_B", "OP_AND_B", "OP_OR_B", "OP_XOR_B",

    "OP_NOT_L", "OP_AND_L", "OP_OR_L",

};

const char *libFuncDecl[][3] = {
    {"getint", "", ": i32"}, {"getch", "", ": i32"}, {"getarray", "*i32", ": i32"},
    {"putint", "i32", ""},   {"putch", "i32", ""},   {"putarray", "*i32, *i32", ""},
    {"starttime", "", ""},   {"stoptime", "", ""},   {NULL, NULL, NULL}};
