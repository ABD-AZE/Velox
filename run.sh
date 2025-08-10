#!/bin/bash

# Iterate over tests directory
test_number=1
for test_file in tests/*; do
	if [ -f "$test_file" ]; then
		cat "$test_file"
		echo -e "\n----------------------------------------"
		echo -e "Running velox on test case $test_number\n"
		# Run the velox binary with the test file
    echo -e "Lexer output:\n $(./velox $test_file)"
		echo -e "\n----------------------------------------"
		((test_number++))
	fi
done
