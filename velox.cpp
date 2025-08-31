#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

int main(int argc, char *argv[]) {
  // if (argc != 2) {
  //   std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
  //   return 1;
  // }

  // std::string source = argv[1];
  std::string source = "test.c";
  if (source.empty()) {
    return 1;
  }
  Lexer lexer(source);
  lexer.GenerateTokens();
  if (!lexer.success) {
    std::cerr << "Lexical analysis failed with errors." << std::endl;
    lexer.PrintErrors();
    return 1;
  }

  Parser parser(lexer.GetTokens());
  parser.parseProgram();

  lexer.PrintTokens();
  return 0;
}