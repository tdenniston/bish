#ifndef __BISH_COMPILE_H__
#define __BISH_COMPILE_H__

#include <iostream>
#include "IR.h"

namespace Bish {

class CodeGenerator;

void compile_to(Module *m, Bish::CodeGenerator *c);

}


#endif
