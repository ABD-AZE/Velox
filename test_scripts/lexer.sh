#!/bin/bash
# Google Test runner for Velox lexer tests

# Colors for pretty printing
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}========================================${NC}"
echo -e "${YELLOW}Velox Lexer Google Test Runner${NC}"
echo -e "${CYAN}========================================${NC}"

# Check if Google Test is installed
if ! pkg-config --exists gtest; then
    echo -e "${RED}Error: Google Test not found. Please install it:${NC}"
    echo -e "${YELLOW}Ubuntu/Debian: sudo apt-get install libgtest-dev${NC}"
    echo -e "${YELLOW}Fedora: sudo dnf install gtest-devel${NC}"
    echo -e "${YELLOW}Arch: sudo pacman -S gtest${NC}"
    echo -e "${YELLOW}Or build from source: https://github.com/google/googletest${NC}"
    exit 1
fi

# Build the tests
echo -e "\n${BLUE}Building Google Tests...${NC}"
if (make test); then
    echo -e "${GREEN}✓ Build successful!${NC}"
else
    echo -e "${RED}✗ Build Failed!${NC}"
    exit 1
fi

# Run lexer tests
echo -e "\n${BLUE}Running Lexer Tests...${NC}"
if (./build/lexer_test); then
    echo -e "${GREEN}✓ Tests completed successfully!${NC}"
else
    echo -e "${RED}✗ Tests failed!${NC}"
    exit 1
fi