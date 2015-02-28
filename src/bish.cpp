#include <iostream>
#include "Parser.h"

int main() {
  Bish::Parser p;
  std::string test = "{ a = 0; b = 7; c = a + b; }";
  Bish::AST *ast = p.parse(test);
  Bish::BishPrinter printer(std::cout);
  ast->accept(&printer);
  return 0;
}
