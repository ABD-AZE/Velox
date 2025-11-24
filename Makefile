CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra
DEBUGFLAGS = -std=c++20 -Wall -Wextra -g -O0
GTEST_FLAGS = -lgtest -lgtest_main -pthread
TEST_TARGETS = build/lexer_test
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# Object file lists
OBJ_FILES = build/obj/lexer.o build/obj/velox.o build/obj/token.o build/obj/parser.o build/obj/ast_printer.o build/obj/valor.o build/obj/semantic_analysis.o build/obj/codegen.o build/obj/symbol_table.o build/obj/ast.o
DEBUG_OBJ_FILES = build/obj/debug/lexer.o build/obj/debug/velox.o build/obj/debug/token.o build/obj/debug/parser.o build/obj/debug/ast_printer.o build/obj/debug/valor.o build/obj/debug/semantic_analysis.o build/obj/debug/codegen.o build/obj/debug/symbol_table.o build/obj/debug/ast.o

.PHONY: all clean help test install uninstall 

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
build/obj/ast.o: ast/ast.cpp
	@mkdir -p build/obj 
	$(CXX) $(CXXFLAGS) -c $< -o $@

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

build/obj/symbol_table.o: symbol_table/symbol_table.cpp
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
build/obj/debug/ast.o: ast/ast.cpp
	@mkdir -p build/obj/debug
	$(CXX) $(DEBUGFLAGS) -c $< -o $@

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

build/obj/debug/symbol_table.o: symbol_table/symbol_table.cpp
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

install: build/velox
	@echo "Installing velox to $(BINDIR)..."
	@mkdir -p $(BINDIR)
	install -m 755 build/velox $(BINDIR)/velox
	@echo "Velox installed successfully to $(BINDIR)/velox"
	@echo "You can now run 'velox' from anywhere"

uninstall:
	@echo "Uninstalling velox from $(BINDIR)..."
	rm -f $(BINDIR)/velox
	@echo "Velox uninstalled successfully"

clean:
	rm -rf build

help:
	@echo "Available targets:"
	@echo "  all       - Build the velox executable"
	@echo "  velox     - Build the velox executable"
	@echo "  debug     - Build the debug version"
	@echo "  test      - Build and run Google Tests"
	@echo "  install   - Install velox to $(BINDIR) (may require sudo)"
	@echo "  uninstall - Remove velox from $(BINDIR) (may require sudo)"
	@echo "  clean     - Remove build files"
	@echo "  help      - Show this help"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX    - Installation prefix (default: /usr/local)"
	@echo "              Usage: make install PREFIX=/custom/path"
