CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g
GTEST_FLAGS = -lgtest -lgtest_main -pthread
TEST_TARGETS = build/lexer_test

.PHONY: all clean help test

all: build/velox build/debug

test: $(TEST_TARGETS)
	@echo "All test files compiled successfully"

build/velox: build/obj/lexer.o build/obj/velox.o
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^

build/debug: build/obj/lexer.o build/obj/velox.o
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -g -o $@ $^

build/obj/lexer.o: lexer/lexer.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/obj/velox.o: velox.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -Ilexer -c $< -o $@

build/lexer_test: build/obj/lexer.o tests/lexer_test.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -Ilexer -Itoken tests/lexer_test.cpp build/obj/lexer.o -o $@ $(GTEST_FLAGS)

clean:
	rm -rf build

help:
	@echo "Available targets:"
	@echo "  all    - Build the velox executable"
	@echo "  velox  - Build the velox executable"
	@echo "  test   - Build and run Google Tests"
	@echo "  clean  - Remove build files"
	@echo "  help   - Show this help"
