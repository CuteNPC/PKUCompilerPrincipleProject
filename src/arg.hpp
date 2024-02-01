#ifndef _ARG_HPP_
#define _ARG_HPP_

#include <cstdio>
#include <iostream>
#include <fstream>

class Arg
{
private:
    bool _toRISCV;
    FILE *_inputFile;
    std::ostream *_ostreamPtr;
    std::ofstream _ostream;

public:
    Arg(int argc, const char *argv[]);
    FILE *inputFile() const;
    std::ostream &ostream() const;
    bool toRISCV() const;
    ~Arg();
};

#endif // !_ARG_HPP