#include "CodeGen_Bash.h"

namespace Bish {

template<typename T>
static CodeGenerator* create_instance(std::ostream &os)
{
    return new T(os);
}

CodeGenerators::CodeGeneratorsMap CodeGenerators::generator_map;

void CodeGenerators::initialize()
{
    /*
      We are saving a function pointer which will construct
      the actual code generator object when needed.
    */
    generator_map["bash"] = &create_instance<CodeGen_Bash>;
}

const CodeGenerators::CodeGeneratorsMap& CodeGenerators::all()
{
    return generator_map;
}

CodeGenerators::CodeGeneratorConstructor CodeGenerators::get(const std::string &name)
{
    CodeGeneratorsMap::iterator it = generator_map.find(name);
    if (it == generator_map.end()) {
        return NULL;
    }

    return it->second;
}

}
