file build/debug
set args tests/parser_tests/test1.vlx --ir
set breakpoint pending on
break velox.cpp:94
break semantic_analysis.cpp:90