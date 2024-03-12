#ifndef _ARG_HPP_
#define _ARG_HPP_

#include <cstdio>
#include <fstream>
#include <iostream>

class Arg
{
  private:
    bool _toRiscv;
    FILE *_inputFile;
    std::ostream *_ostreamPtr;
    std::ofstream _ostream;

  public:
    Arg(int argc, const char *argv[]);
    FILE *inputFile() const;
    std::ostream &ostream() const;
    bool toRiscv() const;
    bool toKoopa() const;
    ~Arg();
};

#endif // !_ARG_HPP