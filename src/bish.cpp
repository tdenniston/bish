#include <stdio.h>
#include <limits.h>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <set>
#include <string>
#include <queue>
#include <iostream>
#include "CallGraph.h"
#include "CodeGen_Bash.h"
#include "Parser.h"
#include "IRVisitor.h"

const std::string BISH_VERSION = "0.1";
const std::string BISH_URL = "https://github.com/tdenniston/bish";
const std::string STDLIB_PATH = "src/StdLib.bish";

class FindFunctionCalls : public Bish::IRVisitor {
public:
    FindFunctionCalls(const std::set<std::string> &n) {
        to_find.insert(n.begin(), n.end());
    }

    std::set<std::string> names() { return names_; }

    virtual void visit(Bish::FunctionCall *call) {
        for (std::vector<Bish::IRNode *>::const_iterator I = call->args.begin(),
                 E = call->args.end(); I != E; ++I) {
            (*I)->accept(this);
        }
        if (to_find.count(call->function->name)) {
            names_.insert(call->function->name);
        }
    }
private:
    std::set<std::string> to_find;
    std::set<std::string> names_;
};

// Return the path to the standard library. This tries a couple of
// options before falling back on the hardcoded value.
std::string get_stdlib_path() {
    char abspath[PATH_MAX];
    char *root = std::getenv("BISH_ROOT");
    char *stdlib = std::getenv("BISH_STDLIB");
    if (root) {
        root = realpath(root, abspath);
        assert(root);
        return std::string(abspath) + "/" + STDLIB_PATH;
    } else if (stdlib) {
        stdlib = realpath(stdlib, abspath);
        assert(stdlib);
        return std::string(abspath);
    } else {
        return STDLIB_PATH;
    }
}

// Add necessary stdlib functions to the given module.
void link_stdlib(Bish::Module *m) {
    Bish::Parser p;
    Bish::Module *stdlib = p.parse(get_stdlib_path());
    std::set<std::string> stdlib_functions;
    for (std::vector<Bish::Function *>::iterator I = stdlib->functions.begin(),
             E = stdlib->functions.end(); I != E; ++I) {
        Bish::Function *f = *I;
        if (f->name.compare("main") == 0) continue;
        stdlib_functions.insert(f->name);
    }
    FindFunctionCalls find(stdlib_functions);
    m->accept(&find);
    Bish::CallGraphBuilder cgb;
    Bish::CallGraph cg = cgb.build(stdlib);
    
    std::set<std::string> to_link = find.names();
    for (std::set<std::string>::iterator I = to_link.begin(), E = to_link.end(); I != E; ++I) {
        Bish::Function *f = stdlib->get_function(*I);
        assert(f);
        if (f->name.compare("main") == 0) continue;
        m->add_function(f);
        std::vector<Bish::Function *> calls = cg.transitive_calls(f);
        for (std::vector<Bish::Function *>::iterator II = calls.begin(), EE = calls.end(); II != EE; ++II) {
            m->add_function(stdlib->get_function((*II)->name));
        }
    }
}

void compile_to_bash(std::ostream &os, Bish::Module *m) {
    link_stdlib(m);
    os << "#!/usr/bin/env bash\n\n";
    os << "# Autogenerated script, compiled from the Bish language.\n";
    os << "# Bish version " << BISH_VERSION << "\n";
    os << "# Please see " << BISH_URL << " for more information about Bish.\n\n";
    Bish::CodeGen_Bash compile(os);
    m->accept(&compile);
}

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
        compile_to_bash(s, m);
        run_on_bash(s);
    } else {
        std::string path(argv[1]);
        Bish::Parser p;
        Bish::Module *m = p.parse(path);

        compile_to_bash(std::cout, m);
    }

    return 0;
}
