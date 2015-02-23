CXX=g++
CFLAGS=-g -O0
RM=rm -f

SRC=src
OBJ=obj

all: bish

parser.o: $(SRC)/parser.cpp
	$(CXX) $(CFLAGS) -c $(SRC)/parser.cpp -o $(OBJ)/parser.o

AST.o: $(SRC)/AST.cpp
	$(CXX) $(CFLAGS) -c $(SRC)/AST.cpp -o $(OBJ)/AST.o

bish.o: $(SRC)/bish.cpp
	$(CXX) $(CFLAGS) -c $(SRC)/bish.cpp -o $(OBJ)/bish.o

bish: dirs parser.o bish.o AST.o
	$(CXX) $(CFLAGS) -o bish $(OBJ)/parser.o $(OBJ)/bish.o $(OBJ)/AST.o

dirs:
	mkdir -p $(OBJ)

clean:
	$(RM) $(OBJ)/* bish
	rmdir $(OBJ)
