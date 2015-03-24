CXX?=c++
CXXFLAGS?=-g -O0
RM=rm -f

SRC=src
OBJ=obj
BIN=/usr/bin

SOURCE_FILES=CallGraph.cpp CodeGen.cpp CodeGen_Bash.cpp Compile.cpp FindCalls.cpp IR.cpp IRAncestorsPass.cpp IRVisitor.cpp LinkImportsPass.cpp Parser.cpp SymbolTable.cpp Tokenizer.cpp TypeChecker.cpp Util.cpp
HEADER_FILES=CallGraph.h CodeGen.h CodeGen_Bash.h Compile.h FindCalls.h IR.h IRAncestorsPass.h IRVisitor.h LinkImportsPass.h Parser.h SymbolTable.h Tokenizer.h TypeChecker.h Util.h

OBJECTS = $(SOURCE_FILES:%.cpp=$(OBJ)/%.o)
HEADERS = $(HEADER_FILES:%.h=$(SRC)/%.h)

ROOT_DIR = $(realpath $(dir $(firstword $(MAKEFILE_LIST))))
CONFIG_CONSTANTS = -DSTDLIB_PATH="\"$(ROOT_DIR)/src/StdLib.bish\""

all: bish

-include $(OBJ)/*.d

$(OBJ)/%.o: $(SRC)/%.cpp $(SRC)/%.h
	@-mkdir -p $(OBJ)
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MF $(OBJ)/$*.d -MT $(OBJ)/$*.o $(CONFIG_CONSTANTS)

$(OBJ)/libbish.a: $(OBJECTS)
	$(LD) -r -o $(OBJ)/bish.o $(OBJECTS)
	ar -ru $@ $(OBJ)/bish.o
	ranlib $@

bish: $(SRC)/bish.cpp $(OBJ)/libbish.a
	$(CXX) $(CXXFLAGS) -o bish $(SRC)/bish.cpp $(OBJ)/libbish.a $(CONFIG_CONSTANTS)

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
