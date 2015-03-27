#ifndef __BISH_COMPILE_H__
#define __BISH_COMPILE_H__

#include <iostream>
#include "CodeGen.h"
#include "IR.h"

namespace Bish {

void compile_to(Module *m, Bish::CodeGenerator *c, bool compile_as_library);

}

#endif
