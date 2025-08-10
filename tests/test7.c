"a" "hello" "quote: \" slash: \\ newline:\n tab:\t"
'c' '\n' '\t' '\'' '\"' '\\'
"a
"        // this will trigger error recovery on unterminated string (newline)
'a        // this will trigger error recovery on unterminated char (EOF or newline)
