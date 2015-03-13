#include <stdio.h>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <iostream>
#include "Compile.h"
#include "Parser.h"

void run_on_bash(std::istream &is) {
    FILE *bash = popen("bash", "w");
    char buf[4096];

    do {
        is.read(buf, sizeof(buf));
        fwrite(buf, 1, is.gcount(), bash);
    } while (is.gcount() > 0);

    fflush(bash);
    pclose(bash);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "USAGE: " << argv[0] << " [-r] <INPUT>\n";
        std::cerr << "  Compiles Bish file <INPUT> to bash.\n";
        std::cerr << "\nOPTIONS:\n";
        std::cerr << "  -r  compiles and runs the file.\n";
        return 1;
    }

    if (strcmp(argv[1], "-r") == 0) {
        if (argc != 3) {
            std::cerr << "-r needs a filename\n";
            return 1;
        }

        std::string path(argv[2]);
        Bish::Parser p;
        Bish::Module *m = p.parse(path);

        std::stringstream s;
        Bish::compile_to_bash(s, m);
        run_on_bash(s);
    } else {
        std::string path(argv[1]);
        Bish::Parser p;
        Bish::Module *m = p.parse(path);

        compile_to_bash(std::cout, m);
    }

    return 0;
}
