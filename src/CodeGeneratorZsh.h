#ifndef CODEGENERATORZSH_H
#define CODEGENERATORZSH_H

#include <iostream>
#include "CodeGen.h"

namespace Bish {

class CodeGeneratorZsh : public CodeGenerator
{
public:
    CodeGeneratorZsh(std::ostream&);
};

}

#endif // CODEGENERATORZSH_H
