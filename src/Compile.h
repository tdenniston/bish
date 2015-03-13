#ifndef __BISH_COMPILE_H__
#define __BISH_COMPILE_H__

#include <iostream>
#include "IR.h"

namespace Bish {

void compile_to_bash(std::ostream &os, Module *m);

}


#endif
