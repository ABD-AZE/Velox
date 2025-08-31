#!/bin/bash
# Colors for pretty printing
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}========================================${NC}"
echo -e "${YELLOW}VELOX C COMPILER - PARSER TEST SUITE${NC}"
echo -e "${CYAN}========================================${NC}"

# Build Velox using make
echo -e "\n${BLUE}Building Velox...${NC}"
if make; then
	echo -e "${GREEN}✓ Build successful!${NC}"
else
	echo -e "${RED}✗ Build failed!${NC}"
	exit 1
fi

# Check if velox binary exists after build
if [ ! -f "build/velox" ]; then
	echo -e "${RED}Error: velox binary not found after build.${NC}"
	exit 1
fi

# Run parser tests from tests/parser-tests/ directory
echo -e "\n${BLUE}Running parser tests...${NC}"
test_number=1
passed_tests=0
total_tests=0

for test_file in tests/parser-tests/*.vlx; do
	if [ -f "$test_file" ]; then
		((total_tests++))
		echo -e "\n${CYAN}========================================${NC}"
		echo -e "${YELLOW}Parser Test #${test_number}: $(basename "$test_file")${NC}"
		echo -e "${BLUE}File: ${test_file}${NC}"
		echo -e "${GREEN}Input:${NC}"
		cat "$test_file"
		echo -e "${CYAN}========================================${NC}"
		
		# Run the velox binary with the test file
		echo -e "\n${PURPLE}Parser Output:${NC}"
		if build/velox "$test_file" 2>&1; then
			echo -e "\n${GREEN}✓ Test passed (exit code: 0)${NC}"
			((passed_tests++))
		else
			exit_code=$?
			echo -e "\n${RED}✗ Test failed (exit code: $exit_code)${NC}"
		fi
		echo -e "${CYAN}========================================${NC}"
		((test_number++))
	fi
done

# Summary
echo -e "\n${CYAN}========================================${NC}"
echo -e "${YELLOW}TEST SUMMARY${NC}"
echo -e "${BLUE}Total tests: ${total_tests}${NC}"
echo -e "${GREEN}Passed: ${passed_tests}${NC}"
echo -e "${RED}Failed: $((total_tests - passed_tests))${NC}"
if [ $passed_tests -eq $total_tests ]; then
	echo -e "${GREEN}✓ All tests passed!${NC}"
else
	echo -e "${RED}✗ Some tests failed.${NC}"
fi
echo -e "${CYAN}========================================${NC}"

