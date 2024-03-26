#include <cstddef>

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
    {"putint", "i32", ""},   {"putch", "i32", ""},   {"putarray", "i32, *i32", ""},
    {"starttime", "", ""},   {"stoptime", "", ""},   {NULL, NULL, NULL}};

const char *riscvInst[] = {
    
    // OP_NONE,
    NULL,

    // OP_PRI,
    NULL,

    // OP_E,
    "eq",
    // OP_NE,
    "ne",
    // OP_L,
    "lt",
    // OP_LE,
    "le",
    // OP_G,
    "gt",
    // OP_GE,
    "ge",

    // OP_POS,
    NULL,
    // OP_NEG,
    "sub 0",
    // OP_ADD,
    "add",
    // OP_SUB,
    "sub",
    // OP_MUL,
    "mul",
    // OP_DIV,
    "div",
    // OP_MOD,
    "mod",

    // OP_NOT_B,
    "xor -1",
    // OP_AND_B,
    "and",
    // OP_OR_B,
    "or",
    // OP_XOR_B,
    "xor",

    // OP_NOT_L,
    "eq 0",
    // OP_AND_L,
    NULL,
    // OP_OR_L,
    NULL
};