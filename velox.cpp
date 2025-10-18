#include "ast/ast_printer.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "valor/valor.hpp"
#include "semantic_analysis/semantic_analysis.hpp"

int main(int argc, char *argv[])
{
  bool lexflag = 0;
  bool parseflag = 0;
  bool irflag = 0;
  bool validateflag =0;
  // if (argc < 2) {
  //   std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
  //   return 1;
  // }
  std::string source = "tests/parser_tests/test1.vlx";
  // set the flags based on --parse and --lex options
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "--parse")
    {
      parseflag = 1;
    }
    else if (arg == "--lex")
    {
      lexflag = 1;
    }
    else if (arg == "--ir")
    {
      irflag = 1;
    }
    else if(arg == "--validate"){
      validateflag=1;
    }
    else
    {
      source = arg;
    }
  }
  if (source.empty())
  {
    return 1;
  }
  // run the preprocessor
  std::string preprocessedfile =
      source.substr(0, source.find_last_of('.')) + ".i";
  std::string command = "gcc -E -P -x c " + source + " -o " + preprocessedfile;
  int ret = system(command.c_str());
  if (ret != 0)
  {
    std::cerr << "Preprocessing failed." << std::endl;
    std::remove(preprocessedfile.c_str());
    return 1;
  }
  source = preprocessedfile;
  Lexer lexer(source);
  lexer.GenerateTokens();
  if (!lexer.success)
  {
    std::cerr << "Lexical analysis failed with errors." << std::endl;
    lexer.PrintErrors();
    std::remove(preprocessedfile.c_str());
    return 1;
  }
  lexer.PrintTokens();
  if (lexflag)
  {
    std::remove(preprocessedfile.c_str());
    return 0;
  }
  Parser parser(lexer.GetTokens());
  ASTNodePtr &ast = parser.parseProgram();
  if (!parser.isSuccessful())
  {
    std::cerr << "Parsing failed with errors." << std::endl;
    parser.printErrors();
    std::remove(preprocessedfile.c_str());
    return 1;
  }

  std::cout << "=== AST (Before Semantic Analysis) ===" << std::endl;
  ASTPrinter::print(ast);

  if (parseflag)
  {
    std::remove(preprocessedfile.c_str());
    return 0;
  }

  // Run semantic analysis
  SemanticAnalyzer semanticAnalyzer;
  semanticAnalyzer.analyze(ast);

  if (!semanticAnalyzer.success)
  {
    std::cerr << "Semantic analysis failed with errors." << std::endl;
    for (const auto &error : semanticAnalyzer.errors)
    {
      std::cerr << "  " << error << std::endl;
    }
    std::remove(preprocessedfile.c_str());
    return 1;
  }

  std::cout << "\n=== AST (After Semantic Analysis) ===" << std::endl;
  ASTPrinter::print(ast);
  if(validateflag){
    std::remove(preprocessedfile.c_str());
    return 0;
  }
  // Generate IR
  Valor valor;
  auto irProgram = valor.convertToIR(ast);
  std::cout << "\n=== IR ===" << std::endl;
  std::cout << irProgram->toString() << std::endl;
  // writing the IR in a file
  std::ofstream irFile("output.ir");
  if (irFile.is_open())
  {
    irFile << irProgram->toString();
    irFile.close();
  }
  if (irflag)
  {
    std::remove(preprocessedfile.c_str());
    return 0;
  }
  std::remove(preprocessedfile.c_str());
  return 0;
}