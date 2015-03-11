CXX=g++
CXXFLAGS?=-g -O0
RM=rm -f

SRC=src
OBJ=obj
BIN=/usr/bin

SOURCE_FILES=CallGraph.cpp IR.cpp IRVisitor.cpp IRAncestorsPass.cpp CodeGen_Bash.cpp Parser.cpp SymbolTable.cpp TypeChecker.cpp
HEADER_FILES=CallGraph.h IR.h IRVisitor.h IRAncestorsPass.h CodeGen_Bash.h Parser.h SymbolTable.h TypeChecker.h

OBJECTS = $(SOURCE_FILES:%.cpp=$(OBJ)/%.o)
HEADERS = $(HEADER_FILES:%.h=$(SRC)/%.h)

all: bish

$(OBJ)/%.o: $(SRC)/%.cpp $(SRC)/%.h
	@-mkdir -p $(OBJ)
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MF $(OBJ)/$*.d -MT $(OBJ)/$*.o

bish: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o bish $(SRC)/bish.cpp $(OBJECTS)

.PHONY: clean
clean:
	$(RM) bish
	$(RM) -r $(OBJ)
	$(RM) -r bish.dSYM

.PHONY: install
install:
	@-cp bish $(BIN)

.PHONY: uninstall
uninstall:
	$(RM) $(BIN)/bish
