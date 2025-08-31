#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "ast/ast_printer.hpp"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
    return 1;
  }

  std::string source = argv[1];
  if (source.empty()) {
    return 1;
  }
  
  std::cout << "=== VELOX C COMPILER - LEXER + PARSER TEST ===" << std::endl;
  std::cout << "Source file: " << source << std::endl << std::endl;
  
  // === LEXICAL ANALYSIS ===
  std::cout << "=== LEXICAL ANALYSIS ===" << std::endl;
  Lexer lexer(source);
  lexer.GenerateTokens();
  if (!lexer.success) {
    std::cerr << "Lexical analysis failed with errors." << std::endl;
    lexer.PrintErrors();
    return 1;
  }
  
  std::cout << "Lexical analysis completed successfully!" << std::endl;
  std::cout << "Generated " << lexer.GetTokens().size() << " tokens." << std::endl;
  
  // Print tokens for debugging
  std::cout << "\n--- TOKENS ---" << std::endl;
  lexer.PrintTokens();
  
  // === SYNTAX ANALYSIS ===
  std::cout << "\n=== SYNTAX ANALYSIS ===" << std::endl;
  Parser parser(lexer.GetTokens());
  auto program_ast = parser.parseProgram();
  
  // Check for parsing errors and display them
  if (!parser.isSuccessful()) {
    std::cout << "Syntax analysis completed with errors:" << std::endl;
    parser.printErrors();
  } else {
    std::cout << "Syntax analysis completed successfully!" << std::endl;
  }
  
  if (!program_ast) {
    std::cerr << "Syntax analysis failed - no AST generated." << std::endl;
    return 1;
  }
  
  std::cout << "Syntax analysis completed successfully!" << std::endl;
  std::cout << "AST generated with " << program_ast->declarations.size() << " declarations." << std::endl;
  
  // === AST OUTPUT ===
  std::cout << "\n=== ABSTRACT SYNTAX TREE ===" << std::endl;
  velox_astpp::PrintAST(program_ast.get(), std::cout);
  
  std::cout << "\n=== COMPILATION COMPLETE ===" << std::endl;
  return 0;
}