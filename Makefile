CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra
DEBUGFLAGS = -std=c++20 -Wall -Wextra -g -O0
GTEST_FLAGS = -lgtest -lgtest_main -pthread
TEST_TARGETS = build/lexer_test

# Object file lists
OBJ_FILES = build/obj/lexer.o build/obj/velox.o build/obj/token.o build/obj/parser.o build/obj/ast_printer.o build/obj/valor.o build/obj/semantic_analysis.o build/obj/codegen.o
DEBUG_OBJ_FILES = build/obj/debug/lexer.o build/obj/debug/velox.o build/obj/debug/token.o build/obj/debug/parser.o build/obj/debug/ast_printer.o build/obj/debug/valor.o build/obj/debug/semantic_analysis.o build/obj/debug/codegen.o

.PHONY: all clean help test 

all: build/velox build/debug

debug: build/debug 	

test: $(TEST_TARGETS)
	@echo "All test files compiled successfully"

build/velox: $(OBJ_FILES)
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^	

build/debug: $(DEBUG_OBJ_FILES)
	@mkdir -p build
	$(CXX) $(DEBUGFLAGS) -o $@ $^

# Object files
build/obj/lexer.o: lexer/lexer.cpp 
	@mkdir -p build/obj 
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/obj/velox.o: velox.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -Ilexer -c $< -o $@

build/obj/token.o: token/token.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/obj/parser.o: parser/parser.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/obj/ast_printer.o: ast/ast_printer.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@	

build/obj/valor.o: valor/valor.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/obj/semantic_analysis.o: semantic_analysis/semantic_analysis.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@	

build/obj/codegen.o: codegen/codegen.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Debug object files
build/obj/debug/lexer.o: lexer/lexer.cpp 
	@mkdir -p build/obj/debug
	$(CXX) $(DEBUGFLAGS) -c $< -o $@

build/obj/debug/velox.o: velox.cpp
	@mkdir -p build/obj/debug
	$(CXX) $(DEBUGFLAGS) -Ilexer -c $< -o $@

build/obj/debug/token.o: token/token.cpp
	@mkdir -p build/obj/debug
	$(CXX) $(DEBUGFLAGS) -c $< -o $@

build/obj/debug/parser.o: parser/parser.cpp
	@mkdir -p build/obj/debug
	$(CXX) $(DEBUGFLAGS) -c $< -o $@

build/obj/debug/ast_printer.o: ast/ast_printer.cpp
	@mkdir -p build/obj/debug
	$(CXX) $(DEBUGFLAGS) -c $< -o $@

build/obj/debug/valor.o: valor/valor.cpp
	@mkdir -p build/obj/debug
	$(CXX) $(DEBUGFLAGS) -c $< -o $@

build/obj/debug/semantic_analysis.o: semantic_analysis/semantic_analysis.cpp
	@mkdir -p build/obj/debug
	$(CXX) $(DEBUGFLAGS) -c $< -o $@

build/obj/debug/codegen.o: codegen/codegen.cpp
	@mkdir -p build/obj/debug
	$(CXX) $(DEBUGFLAGS) -c $< -o $@
	
build/lexer_test: build/obj/lexer.o tests/lexer_test.cpp build/obj/token.o
	@mkdir -p build
	$(CXX) $(CXXFLAGS) $^ -o $@ $(GTEST_FLAGS)

clean:
	rm -rf build

help:
	@echo "Available targets:"
	@echo "  all    - Build the velox executable"
	@echo "  velox  - Build the velox executable"
	@echo "  test   - Build and run Google Tests"
	@echo "  clean  - Remove build files"
	@echo "  help   - Show this help"
