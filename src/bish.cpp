#include <iostream>
#include "CodeGen_Bash.h"
#include "Parser.h"

void compile_to_bash(std::ostream &os, Bish::Module *m) {
    os << "#!/bin/bash\n";
    Bish::CodeGen_Bash compile(os);
    m->accept(&compile);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "USAGE: " << argv[0] << " <INPUT>\n";
        std::cerr << "  Compiles Bish file <INPUT> to bash.\n";
        return 1;
    }
    
    std::string path(argv[1]);
    Bish::Parser p;
    Bish::Module *m = p.parse(path);

    compile_to_bash(std::cout, m);
    
    return 0;
}
