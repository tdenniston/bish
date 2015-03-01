#include <iostream>
#include "CompileToBash.h"
#include "Parser.h"

int main() {
  std::string test = "{ a = 0; b = 7; c = a + b; }";
  Bish::Parser p;
  Bish::AST *ast = p.parse(test);
  Bish::BishPrinter printer(std::cout);
  ast->accept(&printer);
  std::cout << std::endl;

  Bish::CompileToBash compile(std::cout);
  ast->accept(&compile);
  
  return 0;
}
