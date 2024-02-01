#include "arg.hpp"
#include <string>

Arg::Arg(int argc, const char *argv[])
{
    if (argc != 3 && argc != 5)
    {
        std::cerr << "Error args number" << std::endl;
        exit(-1);
    }
    std::string compilerMode = argv[1];
    if (compilerMode == "-koopa")
        _toRISCV = false;
    else if (compilerMode == "-riscv")
        _toRISCV = true;
    else
    {
        std::cerr << "Error compiler mode" << std::endl;
        exit(-1);
    }
    _inputFile = fopen(argv[2], "r");
    if (!_inputFile)
    {
        std::cerr << "Open input file error" << std::endl;
        exit(-1);
    }
    if (argc == 3)
    {
        _ostreamPtr = &(std::cout);
        return;
    }
    std::string outputMode = argv[3];
    if (outputMode != "-o")
    {
        std::cerr << "Error output mode" << std::endl;
        exit(-1);
    }
    _ostream = std::ofstream(argv[4], std::ios::out);
    if (!_ostream.is_open())
    {
        std::cerr << "Open output file error" << std::endl;
        exit(-1);
    }
    _ostreamPtr = &(_ostream);
}

FILE *Arg::inputFile() const
{
    return _inputFile;
}

std::ostream &Arg::ostream() const
{
    return *_ostreamPtr;
}

bool Arg::toRISCV() const
{
    return _toRISCV;
}

Arg::~Arg()
{
    if (_inputFile != NULL)
        fclose(_inputFile);
    if (_ostream.is_open())
        _ostream.close();
}
