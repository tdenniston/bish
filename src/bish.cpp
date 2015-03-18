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
    std::cerr << "  Compiles Bish file <INPUT> to bash. Specifying '-' for <INPUT>\n";
    std::cerr << "  reads from standard input.\n";
    std::cerr << "\nOPTIONS:\n";
    std::cerr << "  -h: Displays this help message.\n";
    std::cerr << "  -r: Compiles and runs the file.\n";
}

int main(int argc, char **argv) {
    int c;
    bool run_after_compile = false;
    while ((c = getopt(argc,argv, "hr")) != -1) {
        switch (c) {
        case 'h':
            usage(argv[0]);
            return 1;
        case 'r':
            run_after_compile = true;
            break;
        default:
            break;
        }
    }

    if (optind >= argc) {
        usage(argv[0]);
        return 1;
    }

    std::string path(argv[optind]);
    Bish::Parser p;
    Bish::Module *m = path.compare("-") == 0 ? p.parse(std::cin) : p.parse(path);

    std::stringstream s;
    Bish::compile_to_bash(run_after_compile ? s : std::cout, m);
    if (run_after_compile) {
        int exit_status = run_on_bash(s);
        exit(exit_status);
    }

    return 0;
}
