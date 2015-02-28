CXX=g++
CFLAGS=-g -O0
RM=rm -f

SRC=src
OBJ=obj

SOURCE_FILES=AST.cpp ASTVisitor.cpp Parser.cpp SymbolTable.cpp TypeChecker.cpp
HEADER_FILES=AST.h ASTVisitor.h Parser.h SymbolTable.h TypeChecker.h

OBJECTS = $(SOURCE_FILES:%.cpp=$(OBJ)/%.o)
HEADERS = $(HEADER_FILES:%.h=$(SRC)/%.h)

all: bish

$(OBJ)/%.o: $(SRC)/%.cpp $(SRC)/%.h
	@-mkdir -p $(OBJ)
	$(CXX) $(CXX_FLAGS) -c $< -o $@ -MMD -MF $(OBJ)/$*.d -MT $(OBJ)/$*.o

bish: $(OBJECTS)
	$(CXX) $(CFLAGS) -o bish $(SRC)/bish.cpp $(OBJECTS)

.PHONY: clean
clean:
	$(RM) -r $(OBJ)
	$(RM) -r bish.dSYM
