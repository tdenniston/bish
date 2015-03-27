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

int run_on(const std::string &sh, std::istream &is) {
    FILE *shell = popen(sh.c_str(), "w");
    char buf[4096];

    do {
        is.read(buf, sizeof(buf));
        fwrite(buf, 1, is.gcount(), shell);
    } while (is.gcount() > 0);

    fflush(shell);

    // pclose returns the exit status of the process,
    // but shifted to the left by 8 bits.
    int e = pclose(shell) >> 8;
    return e;
}

void usage(char *argv0) {
    std::cerr << "USAGE: " << argv0 << " [-r] <INPUT>\n";
    std::cerr << "  Compiles Bish file <INPUT> to bash. Specifying '-' for <INPUT>\n";
    std::cerr << "  reads from standard input.\n";
    std::cerr << "\nOPTIONS:\n";
    std::cerr << "  -h: Displays this help message.\n";
    std::cerr << "  -r: Compiles and runs the file.\n";
    std::cerr << "  -L: Compiles the file as a library.\n";
    std::cerr << "  -l: list all code generators.\n";
    std::cerr << "  -u <NAME>: use code generator <NAME>.\n";
}

void show_generators_list() {
    const Bish::CodeGenerators::CodeGeneratorsMap &cg_map = Bish::CodeGenerators::all();
    for (Bish::CodeGenerators::CodeGeneratorsMap::const_iterator it = cg_map.begin();
         it != cg_map.end(); ++it) {
        std::cout << it->first << std::endl;
    }
}

int main(int argc, char **argv) {
    Bish::CodeGenerators::initialize();

    int c;
    bool run_after_compile = false;
    bool compile_as_library = false;
    std::string code_generator_name = "bash";

    while ((c = getopt(argc,argv, "hrLlu:")) != -1) {
        switch (c) {
        case 'h':
            usage(argv[0]);
            return 1;
        case 'r':
            run_after_compile = true;
            break;
		case 'L':
			compile_as_library = true;
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

    if (optind >= argc) {
        usage(argv[0]);
        return 1;
    }

    std::string path(argv[optind]);
    std::stringstream s;
    Bish::Parser p;
    Bish::Module *m = path.compare("-") == 0 ? p.parse(std::cin) : p.parse(path);

    Bish::CodeGenerators::CodeGeneratorConstructor cg_constructor =
        Bish::CodeGenerators::get(code_generator_name);

    if (cg_constructor == NULL) {
        std::cerr << "No code generator " << code_generator_name << std::endl;
        return 1;
    }

    Bish::compile_to(m, cg_constructor(run_after_compile ? s : std::cout), compile_as_library);
    if (run_after_compile) {
        const int exit_status = run_on(code_generator_name, s);
        exit(exit_status);
    }

    return 0;
}
