CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g

.PHONY: all clean help

all: velox

velox: build/obj/lexer.o build/obj/velox.o
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $^

build/obj/lexer.o: lexer/lexer.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/obj/velox.o: velox.cpp
	@mkdir -p build/obj
	$(CXX) $(CXXFLAGS) -Ilexer -c $< -o $@

clean:
	rm -rf build velox

help:
	@echo "Available targets:"
	@echo "  all    - Build the velox executable"
	@echo "  velox  - Build the velox executable"
	@echo "  clean  - Remove build files"
	@echo "  help   - Show this help"
