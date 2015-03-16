#include "CodeGen_Bash.h"
#include "CodeGeneratorZsh.h"

namespace Bish {

template<typename T>
CodeGenerator* createInstance(std::ostream &os)
{
    return new T(os);
}

CodeGenerators::CodeGeneratorsMap CodeGenerators::generator_map;

void CodeGenerators::initialize()
{
    generator_map["bash"] = &createInstance<CodeGen_Bash>;
    generator_map["zsh"] = &createInstance<CodeGeneratorZsh>;
}

const CodeGenerators::CodeGeneratorsMap& CodeGenerators::all()
{
    return generator_map;
}

CodeGenerators::Cons CodeGenerators::get(const std::string &name)
{
    return generator_map[name];
}

}
