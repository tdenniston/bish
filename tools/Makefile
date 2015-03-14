LEVEL=..

CXX?=c++
CXXFLAGS?=-g -O0
RM=rm -f

SRC=.
OBJ=$(LEVEL)/obj/tools

SOURCE_FILES=TypeAnnotator.cpp

OBJECTS = $(SOURCE_FILES:%.cpp=$(OBJ)/%.o)

BISH_INCLUDE=$(LEVEL)/src
LIBBISH=$(LEVEL)/obj/libbish.a

all: tools

$(OBJ)/%.o: $(SRC)/%.cpp
	@-mkdir -p $(OBJ)
	$(CXX) $(CXXFLAGS) -c $< -o $@ -I$(BISH_INCLUDE) -MMD -MF $(OBJ)/$*.d -MT $(OBJ)/$*.o

TypeAnnotator: $(OBJ)/TypeAnnotator.o $(LIBBISH)
	$(CXX) $(CXXFLAGS) -o $@ $< -I$(BISH_INCLUDE) $(LIBBISH)

tools: TypeAnnotator

.PHONY: clean
clean:
	$(RM) TypeAnnotator
	$(RM) -r $(OBJ)
