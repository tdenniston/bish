#ifndef CODEGEN_H
#define CODEGEN_H

#include <iostream>
#include <map>

#include "IRVisitor.h"


namespace Bish {

class CodeGenerator: public IRVisitor
{
public:
    CodeGenerator(std::ostream &os): stream(os) {}
    std::ostream& ostream() { return stream; }

protected:
    std::ostream &stream;
};

class CodeGenerators
{
public:
    typedef
    CodeGenerator*(*Cons)(std::ostream& aa);

    typedef
    std::map<std::string, Cons > CodeGeneratorsMap;

    static void initialize();

    static const CodeGeneratorsMap& all();
    static Cons get(const std::string& name);
private:
    static CodeGeneratorsMap generator_map;
};

}
#endif // CODEGEN_H
