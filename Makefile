CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g

.PHONY: all clean help

all: build/velox build/debug

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

clean:
	rm -rf build

help:
	@echo "Available targets:"
	@echo "  all    - Build the velox executable"
	@echo "  velox  - Build the velox executable"
	@echo "  clean  - Remove build files"
	@echo "  help   - Show this help"
