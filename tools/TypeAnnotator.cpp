#include <set>
#include <sstream>
#include <iostream>
#include <cassert>
#include "CodeGen.h"
#include "Compile.h"
#include "IR.h"
#include "Type.h"
#include "Parser.h"

using namespace Bish;

class TypeAnnotator : public IRVisitor {
public:
    TypeAnnotator(std::ostream &os) : stream(os), indent_level(0) {}

    void visit(Module *n) {
        if (visited(n)) return;
        visited_set.insert(n);

        for (std::vector<Function *>::const_iterator I = n->functions.begin(),
                 E = n->functions.end(); I != E; ++I) {
            (*I)->accept(this);
        }
        for (std::vector<Assignment *>::const_iterator I = n->global_variables.begin(),
                 E = n->global_variables.end(); I != E; ++I) {
            (*I)->accept(this);
            stream << ";\n";
        }
    }

    void visit(ReturnStatement *n) {
        if (visited(n)) return;
        visited_set.insert(n);
        stream << "return ";
        n->value->accept(this);
    }

    void visit(Block *n) {
        if (visited(n)) return;
        visited_set.insert(n);
        stream << "{\n";
        indent_level++;
        for (std::vector<IRNode *>::const_iterator I = n->nodes.begin(), E = n->nodes.end();
             I != E; ++I) {
            indent();
            (*I)->accept(this);
            if (!dynamic_cast<IfStatement*>(*I) &&
                !dynamic_cast<ForLoop*>(*I)) stream << ";\n";
        }
        indent_level--;
        indent();
        stream << "}\n\n";
    }

    void visit(Variable *n) {
        stream << n->name.str() << strtype(n);
    }

    void visit(Location *n) {
        n->variable->accept(this);
        if (n->offset) {
            stream << "[";
            n->offset->accept(this);
            stream << "]";
            stream << strtype(n);
        }
    }

    void visit(IfStatement *n) {
        stream << "if (";
        n->pblock->condition->accept(this);
        stream << ") ";
        n->pblock->body->accept(this);

        for (std::vector<PredicatedBlock *>::const_iterator I = n->elses.begin(),
                 E = n->elses.end(); I != E; ++I) {
            indent();
            stream << "else if (";
            (*I)->condition->accept(this);
            stream << ") ";
            (*I)->body->accept(this);
        }
        if (n->elseblock) {
            indent();
            stream << "else ";
            n->elseblock->accept(this);
        }
    }

    void visit(ForLoop *n) {
        stream << "for (";
        n->variable->accept(this);
        stream << " in ";
        n->lower->accept(this);
        if (n->upper) {
            stream << " .. ";
            n->upper->accept(this);
        }
        stream << ") ";
        n->body->accept(this);
    }

    void visit(Function *n) {
        if (visited(n)) return;
        visited_set.insert(n);
        stream << "def " << n->name.str() << strtype(n) << " (";
        const int nargs = n->args.size();
        int i = 0;
        for (std::vector<Variable *>::const_iterator I = n->args.begin(),
                 E = n->args.end(); I != E; ++I, ++i) {
            (*I)->accept(this);
            if (i < nargs - 1) stream << ", ";
        }
        stream << ") ";
        if (n->body) n->body->accept(this);
    }

    void visit(FunctionCall *n) {
        if (visited(n)) return;
        visited_set.insert(n);
        const int nargs = n->args.size();
        stream << n->function->name.str() << "(";
        for (int i = 0; i < nargs; i++) {
            n->args[i]->accept(this);
            if (i < nargs - 1) stream << ", ";
        }
        stream << ")";
    }

    void visit(ExternCall *n) {
        if (visited(n)) return;
        visited_set.insert(n);
        stream << "@(";
        for (InterpolatedString::const_iterator I = n->body->begin(), E = n->body->end();
             I != E; ++I) {
            if ((*I).is_str()) {
                stream << (*I).str();
            } else {
                assert((*I).is_var());
                stream << "$";
                (*I).var()->accept(this);
            }
        }
        stream << ")";
    }

    void visit(IORedirection *n) {
        if (visited(n)) return;
        visited_set.insert(n);
        std::string bash_op;
        switch (n->op) {
        case IORedirection::Pipe:
            bash_op = "|";
            break;
        default:
            assert(false && "Unimplemented redirection.");
        }

        n->a->accept(this);
        stream << " " << bash_op << " ";
        n->b->accept(this);
    }

    void visit(Assignment *n) {
        if (visited(n)) return;
        visited_set.insert(n);
        bool array_init = n->values.size() > 1;
        n->location->accept(this);
        stream << " = ";
        if (array_init) stream << "[";
        const unsigned sz = n->values.size();
        for (unsigned i = 0; i < sz; i++) {
            n->values[i]->accept(this);
            if (i < sz - 1) stream << ", ";
        }
        if (array_init) stream << "]";
    }

    void visit(BinOp *n) {
        std::string bash_op;
        switch (n->op) {
        case BinOp::Eq:
            bash_op = "==";
            break;
        case BinOp::NotEq:
            bash_op = "!=";
            break;
        case BinOp::LT:
            bash_op = "<";
            break;
        case BinOp::LTE:
            bash_op = "<=";
            break;
        case BinOp::GT:
            bash_op = ">";
            break;
        case BinOp::GTE:
            bash_op = ">=";
            break;
        case BinOp::And:
            bash_op = "and";
            break;
        case BinOp::Or:
            bash_op = "or";
            break;
        case BinOp::Add:
            bash_op = "+";
            break;
        case BinOp::Sub:
            bash_op = "-";
            break;
        case BinOp::Mul:
            bash_op = "*";
            break;
        case BinOp::Div:
            bash_op = "/";
            break;
        case BinOp::Mod:
            bash_op = "%";
            break;
        }

        n->a->accept(this);
        stream << " " << bash_op << " ";
        n->b->accept(this);
    }

    void visit(UnaryOp *n) {
        if (visited(n)) return;
        visited_set.insert(n);
        switch (n->op) {
        case UnaryOp::Negate:
            stream << "-";
            break;
        case UnaryOp::Not:
            stream << "!";
            break;
        }
        n->a->accept(this);
    }

    void visit(Integer *n) {
        stream << n->value;
    }

    void visit(Fractional *n) {
        stream << n->value;
    }

    void visit(String *n) {
        stream << "\"" << n->value << "\"";
    }

    void visit(Boolean *n) {
        stream << n->value;
    }
private:
    unsigned indent_level;
    std::ostream &stream;
    std::set<IRNode *> visited_set;
    bool visited(IRNode *n) { return visited_set.find(n) != visited_set.end(); }
    void indent() {
        for (unsigned i = 0; i < indent_level; i++) {
            stream << "    ";
        }
    }
    std::string strtype(IRNode *n) {
        return "{:" + n->type().str() + "}";
    }
};

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "USAGE: " << argv[0] << " <INPUT>\n";
        std::cerr << "  Annotates Bish file <INPUT> with inferred type information.\n";
        return 1;
    }
    std::string path(argv[1]);

    Parser p;
    Module *m = p.parse(path);
    std::stringstream s;
    // Don't actually care about the output, just need the compile
    // pipeline to run.
    CodeGenerators::initialize();
    CodeGenerators::CodeGeneratorConstructor cg_constructor =
        CodeGenerators::get("bash");
    assert(cg_constructor);
    compile_to(m, cg_constructor(s));

    TypeAnnotator annotate(std::cout);
    m->accept(&annotate);

    return 0;
}
