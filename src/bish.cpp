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
#include "CodeGen.h"

void run_on_bash(const std::string& sh, std::istream &is) {
    FILE *bash = popen(sh.c_str(), "w");
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
    std::cerr << "  -l: list all code generators.\n";
    std::cerr << "  -u <name>: use name code generator\n";
}

void show_generators_list() {
    const Bish::CodeGenerators::CodeGeneratorsMap& cg_map = Bish::CodeGenerators::all();
    for (Bish::CodeGenerators::CodeGeneratorsMap::const_iterator it = cg_map.begin();
         it != cg_map.end(); ++it) {
        std::cout << it->first << std::endl;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    Bish::CodeGenerators::initialize();

    int c;
    bool run_after_compile = false;
    std::string code_generator_name = "bash";

    while ((c = getopt(argc,argv, "rlu:")) != -1) {
        switch (c) {
        case 'h':
            usage(argv[0]);
            return 1;
        case 'r':
            run_after_compile = true;
            break;

        case 'l':
            show_generators_list();
            return 1;

        case 'u':
            code_generator_name = std::string(optarg);
            break;

        default:
            break;
        }
    }

    std::string path( optind < argc ? argv[optind] : "");
    if (path.empty()) {
        std::cerr << "please specify input filename" << std::endl;
        return 1;
    }

    std::cout << "using " << code_generator_name << " to compile" << std::endl;

    std::stringstream s;

    Bish::Parser p;
    Bish::Module *m = path.compare("-") == 0 ? p.parse(std::cin) : p.parse(path);

    // cg allocated !
    Bish::CodeGenerator* cg = Bish::CodeGenerators::get(code_generator_name)(run_after_compile ? s : std::cout);

    Bish::compile_to_bash(m, cg);
    if (run_after_compile) {
        const int exit_status = run_on_bash(code_generator_name, s);
        exit(exit_status);
    }

    return 0;
}
