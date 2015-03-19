#include "CodeGen_Bash.h"

namespace Bish {

template<typename T>
static CodeGenerator* createInstance(std::ostream &os)
{
    return new T(os);
}

CodeGenerators::CodeGeneratorsMap CodeGenerators::generator_map;

void CodeGenerators::initialize()
{
    generator_map["bash"] = &createInstance<CodeGen_Bash>;
}

const CodeGenerators::CodeGeneratorsMap& CodeGenerators::all()
{
    return generator_map;
}


CodeGenerators::ConGeneratorConstructor CodeGenerators::get(const std::string &name)
{
    CodeGeneratorsMap::iterator it = generator_map.find(name);
    if (it == generator_map.end()) {
        return NULL;
    }

    return it->second;
}

}
