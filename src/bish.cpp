#include <stdio.h>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include <iostream>
#include <unistd.h>
#include "Compile.h"
#include "Parser.h"

int run_on_bash(std::istream &is) {
    FILE *bash = popen("bash", "w");
    char buf[4096];

    do {
        is.read(buf, sizeof(buf));
        fwrite(buf, 1, is.gcount(), bash);
    } while (is.gcount() > 0);

    fflush(bash);

    // pclose returns the exit status of the process,
    // but shifted to the left by 8 bits.
    int e = pclose(bash) >> 8;
    return e;
}

void usage(char *argv0) {
    std::cerr << "USAGE: " << argv0 << " [-r] <INPUT>\n";
    std::cerr << "  Compiles Bish file <INPUT> to bash.\n";
    std::cerr << "\nOPTIONS:\n";
    std::cerr << "  -r: compiles and runs the file.\n";
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    int c;
    bool run_after_compile = false;
    while ((c = getopt(argc,argv, "r")) != -1) {
        switch (c) {
        case 'r':
            run_after_compile = true;
            break;
        default:
            break;
        }
    }

    if (optind == argc && run_after_compile) {
        std::cerr << "-r needs a filename" << std::endl;
        return 1;
    }

    std::string path(argv[optind]);
    Bish::Parser p;
    Bish::Module *m = p.parse(path);

    std::stringstream s;
    Bish::compile_to_bash(run_after_compile ? s : std::cout, m);
    if (run_after_compile) {
        int exit_status = 0;
        exit_status = run_on_bash(s);
        exit(exit_status);
    }

    return 0;
}
