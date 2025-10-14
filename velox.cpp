#include "ast/ast_printer.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "valor/valor.hpp"

int main(int argc, char *argv[]) {
  bool lexflag = 0;
  bool parseflag = 0;
  bool irflag = 0;
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
    return 1;
  }
  std::string source = "";
  // set the flags based on --parse and --lex options
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--parse") {
      parseflag = 1;
    } else if (arg == "--lex") {
      lexflag = 1;
    } else if (arg == "--ir") {
      irflag = 1;
    } else {
      source = arg;
    }
  }
  if (source.empty()) {
    return 1;
  }
  // preprocess the source file to handle includes
  std::ifstream file(source);
  if (!file.is_open()) {
    std::cerr << "Could not open source file: " << source << std::endl;
    return 1;
  }
  // running gcc -E -P INPUT_FILE -o PREPROCESSED_FILE
  std::string preprocessedFile =
      source.substr(0, source.find_last_of('.')) + ".i";
  std::string command = "gcc -E -P " + source + " -o " + preprocessedFile;
  int ret = system(command.c_str());
  if (ret != 0) {
    std::cerr << "Preprocessing failed." << std::endl;
    return 1;
  }
  source = preprocessedFile;
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

  std::cout << "=== AST ===" << std::endl;
  ASTPrinter::print(ast);

  if (parseflag) {
    return 0;
  }

  // Generate IR
  Valor valor;
  auto irProgram = valor.convertToIR(ast);
  std::cout << "\n=== IR ===" << std::endl;
  std::cout << irProgram->toString() << std::endl;
  // writing the IR in a file
  std::ofstream irFile("output.ir");
  if (irFile.is_open()) {
    irFile << irProgram->toString();
    irFile.close();
  }
  if (irflag) {
    return 0;
  }

  return 0;
}