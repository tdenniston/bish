#ifndef __BISH_CODEGEN_H__
#define __BISH_CODEGEN_H__

#include <iostream>
#include <map>
#include "IRVisitor.h"
#include "LineOrientedBuffer.h"

namespace Bish {

class CodeGenerator: public IRVisitor {
public:
    CodeGenerator(std::ostream &os): stream(os) {}
    LineOrientedBuffer &ostream() { return stream; }

protected:
    LineOrientedBuffer stream;
};

class CodeGenerators {
public:
    // Starting with C++11, std::function can be used.
    typedef CodeGenerator*(*CodeGeneratorConstructor)(std::ostream &aa);
    typedef std::map<std::string, CodeGeneratorConstructor > CodeGeneratorsMap;

    static void initialize();
    static const CodeGeneratorsMap &all();
    static CodeGeneratorConstructor get(const std::string &name);
private:
    static CodeGeneratorsMap generator_map;
};

}
#endif
