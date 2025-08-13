#!/bin/bash
# Colors for pretty printing
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Check if velox binary exists
if [ ! -f "build/velox" ]; then
	echo -e "${RED}Error: velox binary not found in build directory. Run 'make' first.${NC}"
	exit 1
fi

# Iterate over tests directory
test_number=1
for test_file in tests/*; do
	if [ -f "$test_file" ]; then
		echo -e "\n${CYAN}========================================${NC}"
		echo -e "${YELLOW}Test Case #${test_number}${NC}"
		echo -e "${BLUE}File: ${test_file}${NC}"
		echo -e "${GREEN}Input: $(cat "$test_file")${NC}"
		echo -e "${CYAN}========================================${NC}"
		# Run the velox binary with the test file
		echo -e "\n${PURPLE}Lexer Output:${NC}"
		build/velox "./$test_file" 2>&1
		echo -e "\n${CYAN}Return code: $?${NC}"
		echo -e "${CYAN}========================================${NC}"
		((test_number++))
	fi
done
