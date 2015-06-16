#ifndef __BISH_BUILTINS_H__
#define __BISH_BUILTINS_H__

#include <map>
#include <vector>
#include "IR.h"

namespace Bish {

class Builtins {
public:
    Builtins() {
        add("args", Type::Array(Type::String()));
    }

    const std::vector<Name> &names() const { return builtin_symbols; }
    const Type &type(const Name &n) { return *builtin_types[n]; }
private:
    std::vector<Name> builtin_symbols;
    std::map<Name, Type*> builtin_types;

    void add(const std::string &name, const Type &type) {
        Name n(name);
        builtin_symbols.push_back(n);
        builtin_types[n] = new Type(type);
    }
};

// Singleton instance.
static Builtins builtins;

}

#endif
