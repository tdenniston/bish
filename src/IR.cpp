#include <iostream>
#include "IR.h"

namespace Bish {

void Module::set_main(Function *f) {
    add_function(f);
    main = f;
}

void Module::add_function(Function *f) {
    functions.push_back(f);
}
  
}
