#include "koopaparser.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

KoopaParser::KoopaParser() : program(NULL), ret(-1), builder(0), available(false)
{
    memset(&raw, 0, sizeof(raw));
}

KoopaParser::KoopaParser(const std::string &koopaIRSting)
    : program(NULL), ret(-1), builder(0), available(false)
{
    memset(&raw, 0, sizeof(raw));
    buildFrom(koopaIRSting);
}

void KoopaParser::buildFrom(const std::string &koopaIRSting)
{
    ret = koopa_parse_from_string(koopaIRSting.c_str(), &program);

    checkErrorWhileBuild();

    builder = koopa_new_raw_program_builder();

    raw = koopa_build_raw_program(builder, program);

    koopa_delete_program(program);
}

koopa_raw_program_t *KoopaParser::getRaw() { return &(raw); }

KoopaParser::~KoopaParser() { koopa_delete_raw_program_builder(builder); }

void KoopaParser::checkErrorWhileBuild()
{
    switch (ret)
    {
    case KOOPA_EC_SUCCESS:
        // std::cerr << ("No errors occurred.") << std::endl;
        break;
    case KOOPA_EC_INVALID_UTF8_STRING:
        std::cerr << ("UTF-8 string conversion error.") << std::endl;
        break;
    case KOOPA_EC_INVALID_FILE:
        std::cerr << ("File operation error.") << std::endl;
        break;
    case KOOPA_EC_INVALID_KOOPA_PROGRAM:
        std::cerr << ("Koopa IR program parsing error.") << std::endl;
        break;
    case KOOPA_EC_IO_ERROR:
        std::cerr << ("IO operation error.") << std::endl;
        break;
    case KOOPA_EC_NULL_BYTE_ERROR:
        std::cerr << ("Byte array to C string conversion error.") << std::endl;
        break;
    case KOOPA_EC_INSUFFICIENT_BUFFER_LENGTH:
        std::cerr << ("Insufficient buffer length.") << std::endl;
        break;
    case KOOPA_EC_RAW_SLICE_ITEM_KIND_MISMATCH:
        std::cerr << ("Mismatch of item kind in raw slice.") << std::endl;
        break;
    case KOOPA_EC_NULL_POINTER_ERROR:
        std::cerr << ("Passing null pointers to `libkoopa`.") << std::endl;
        break;
    case KOOPA_EC_TYPE_MISMATCH:
        std::cerr << ("Mismatch of type.") << std::endl;
        break;
    case KOOPA_EC_FUNC_PARAM_NUM_MISMATCH:
        std::cerr << ("Mismatch of function parameter number.") << std::endl;
        break;
    default:
        std::cerr << ("Unknown error type.") << std::endl;
        break;
    }

    available = (ret == KOOPA_EC_SUCCESS);
    assert(ret == KOOPA_EC_SUCCESS);
}

/* END */