#include "ast/ast_printer.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic_analysis/semantic_analysis.hpp"
#include "valor/valor.hpp"
#include "codegen/codegen.hpp"

void cleanup(const std::string &filename);

int main(int argc, char *argv[]) {
  bool lexflag = 0;
  bool parseflag = 0;
  bool irflag = 0;
  bool validateflag = 0;
  bool codegenflag = 0;
  bool asmflag = 0;
  // if (argc < 2) {
  //   std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
  //   return 1;
  // }
  std::string source = "tests/parser_tests/test1.vlx";
  // set the flags based on --parse and --lex options
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--parse") {
      parseflag = 1;
    } else if (arg == "--lex") {
      lexflag = 1;
    } else if (arg == "--ir") {
      irflag = 1;
    } else if (arg == "--validate") {
      validateflag = 1;
    } else if (arg == "--codegen") {
      codegenflag = 1;
    } else if (arg == "-S"){
      asmflag = 1;
    } 
    else {
      source = arg;
    }
  }
  if (source.empty()) {
    return 1;
  }
  // run the preprocessor
  std::string preprocessedfile =
      source.substr(0, source.find_last_of('.')) + ".i";
  std::string command = "gcc -E -P -x c " + source + " -o " + preprocessedfile;
  int ret = system(command.c_str());
  if (ret != 0) {
    std::cerr << "Preprocessing failed." << std::endl;
    cleanup(preprocessedfile);
    return 1;
  }
  source = preprocessedfile;
  Lexer lexer(source);
  lexer.GenerateTokens();
  if (!lexer.success) {
    std::cerr << "Lexical analysis failed with errors." << std::endl;
    lexer.PrintErrors();
    cleanup(preprocessedfile);
    return 1;
  }
  lexer.PrintTokens();
  if (lexflag) {
    cleanup(preprocessedfile);
    return 0;
  }


  Parser parser(lexer.GetTokens());
  ASTNodePtr &ast = parser.parseProgram();
  if (!parser.isSuccessful()) {
    std::cerr << "Parsing failed with errors." << std::endl;
    parser.printErrors();
    cleanup(preprocessedfile);
    return 1;
  }

  std::cout << "=== AST (Before Semantic Analysis) ===" << std::endl;
  ASTPrinter::print(ast);

  if (parseflag) {
    cleanup(preprocessedfile);
    return 0;
  }

  // Run semantic analysis
  SemanticAnalyzer semanticAnalyzer;
  semanticAnalyzer.analyze(ast);

  if (!semanticAnalyzer.success) {
    std::cerr << "Semantic analysis failed with errors." << std::endl;
    for (const auto &error : semanticAnalyzer.errors) {
      std::cerr << "  " << error << std::endl;
    }
    cleanup(preprocessedfile);
    return 1;
  }

  std::cout << "\n=== AST (After Semantic Analysis) ===" << std::endl;
  ASTPrinter::print(ast);
  if (validateflag) {
    cleanup(preprocessedfile);
    return 0;
  }

  // Generate IR
  Valor valor(semanticAnalyzer.label_counter);
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
    cleanup(preprocessedfile);
    return 0;
  }

  // Code generation 
  Codegen codegenerator;
  auto asmProgram = codegenerator.generateCode(irProgram);
  if(codegenflag){
    cleanup(preprocessedfile);
    return 0;
  }
  std::string asmfile;
  asmfile = source.substr(0, source.find_last_of('.')) + ".s";
  std::ofstream asmOutputFile(asmfile);
  if (asmOutputFile.is_open()) {
    asmOutputFile << asmProgram->toString();
    asmOutputFile.close();
    std::cout << "Assembly code written to " << asmfile << std::endl;
  } else{
    std::cerr << "Failed to write assembly code to " << asmfile << std::endl;
  }

  if (asmflag) {
    cleanup(preprocessedfile);
    return 0;
  }
  auto cmd = "gcc -m64 " + asmfile + " -o " +
             source.substr(0, source.find_last_of('.'));
  ret = system(cmd.c_str());
  if (ret != 0) { 
    std::cerr << "Assembly failed." << std::endl;
  }
  cleanup(preprocessedfile);
  return 0;
}

void cleanup(const std::string &filename) {
  if (std::remove(filename.c_str()) != 0) {
    std::cerr << "Error deleting file: " << filename << std::endl;
  }
}