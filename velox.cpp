#include "ast/ast_printer.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"

int main(int argc, char *argv[]) {
  bool lexflag = 0;
  bool parseflag = 0;
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
    return 1;
  }
  std::string source;
  // set the flags based on --parse and --lex options
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--parse") {
      parseflag = 1;
    } else if (arg == "--lex") {
      lexflag = 1;
    } else {
      source = arg;
    }
  }
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
  lexer.PrintTokens();
  if (lexflag) {
    return 0;
  }
  Parser parser(lexer.GetTokens());
  const ASTNodePtr &ast = parser.parseProgram();
  if (!parser.isSuccessful()) {
    std::cerr << "Parsing failed with errors." << std::endl;
    parser.printErrors();
    return 1;
  }
  ASTPrinter::print(ast);
  return 0;
}