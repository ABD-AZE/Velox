#include "ast/ast_printer.hpp"
#include "codegen/codegen.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "semantic_analysis/semantic_analysis.hpp"
#include "valor/valor.hpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct CompilerOptions {
  std::string inputFile;
  std::string outputFile;
  bool stopAfterLex = false;
  bool stopAfterParse = false;
  bool stopAfterValidate = false;
  bool stopAfterIR = false;
  bool stopAfterCodegen = false;
  bool stopAfterAsm = false;
  bool stopAfterObject = false;
  bool verbose = false;
  std::vector<std::string> linkLibs;
};

void printUsage(const char *progName) {
  std::cerr << "Usage: " << progName << " [options] <source-file>\n\n";
  std::cerr << "Options:\n";
  std::cerr << "  -o <file>      Place output in <file>\n";
  std::cerr << "  -S             Compile only; generate assembly file\n";
  std::cerr << "  -c             Compile and assemble; generate object file\n";
  std::cerr << "  -l<library>    Link with library\n";
  std::cerr << "  --lex          Stop after lexical analysis\n";
  std::cerr << "  --parse        Stop after parsing\n";
  std::cerr << "  --validate     Stop after semantic analysis\n";
  std::cerr << "  --tacky        Stop after IR generation\n";
  std::cerr << "  --codegen      Stop after code generation\n";
  std::cerr << "  -v, --verbose  Enable verbose output\n";
  std::cerr << "  -h, --help     Display this help message\n";
}

bool parseArguments(int argc, char *argv[], CompilerOptions &opts) {
  if (argc < 2) {
    return false;
  }

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      return false;
    } else if (arg == "-v" || arg == "--verbose") {
      opts.verbose = true;
    } else if (arg == "--lex") {
      opts.stopAfterLex = true;
    } else if (arg == "--parse") {
      opts.stopAfterParse = true;
    } else if (arg == "--validate") {
      opts.stopAfterValidate = true;
    } else if (arg == "--tacky") {
      opts.stopAfterIR = true;
    } else if (arg == "--codegen") {
      opts.stopAfterCodegen = true;
    } else if (arg == "-S") {
      opts.stopAfterAsm = true;
    } else if (arg == "-c") {
      opts.stopAfterObject = true;
    } else if (arg == "-o") {
      if (i + 1 < argc) {
        opts.outputFile = argv[++i];
      } else {
        std::cerr << "Error: -o requires an argument\n";
        return false;
      }
    } else if (arg.substr(0, 2) == "-l") {
      opts.linkLibs.push_back(arg);
    } else if (arg[0] == '-') {
      std::cerr << "Error: Unknown option: " << arg << "\n";
      return false;
    } else {
      if (!opts.inputFile.empty()) {
        std::cerr << "Error: Multiple input files specified\n";
        return false;
      }
      opts.inputFile = arg;
    }
  }

  if (opts.inputFile.empty()) {
    std::cerr << "Error: No input file specified\n";
    return false;
  }

  return true;
}

std::string getBaseName(const std::string &path) {
  size_t lastDot = path.find_last_of('.');

  if (lastDot != std::string::npos) {
    return path.substr(0, lastDot);
  }
  return path;
}

void cleanupFile(const std::string &filename) { std::remove(filename.c_str()); }

int main(int argc, char *argv[]) {
  CompilerOptions opts;

  if (!parseArguments(argc, argv, opts)) {
    printUsage(argv[0]);
    return 1;
  }

  std::string baseName = getBaseName(opts.inputFile);
  std::string preprocessedFile = baseName + ".i";
  std::string asmFile = baseName + ".s";
  std::string objFile = baseName + ".o";
  std::string exeFile = opts.outputFile.empty() ? baseName : opts.outputFile;

  // Preprocessing
  std::string ppCmd =
      "gcc -E -P -x c " + opts.inputFile + " -o " + preprocessedFile + " 2>&1";
  int ret = system(ppCmd.c_str());
  if (ret != 0) {
    std::cerr << "velox: error: preprocessing failed\n";
    cleanupFile(preprocessedFile);
    return 1;
  }

  // Lexical Analysis
  Lexer lexer(preprocessedFile);
  lexer.GenerateTokens();

  if (!lexer.success) {
    std::cerr << "velox: error: lexical analysis failed\n";
    lexer.PrintErrors();
    cleanupFile(preprocessedFile);
    return 1;
  }

  if (opts.verbose) {
    lexer.PrintTokens();
  }

  if (opts.stopAfterLex) {
    cleanupFile(preprocessedFile);
    return 0;
  }

  // Parsing
  Parser parser(lexer.GetTokens());
  ASTNodePtr &ast = parser.parseProgram();

  if (!parser.isSuccessful()) {
    std::cerr << "velox: error: parsing failed\n";
    parser.printErrors();
    cleanupFile(preprocessedFile);
    return 1;
  }

  if (opts.verbose) {
    std::cout << "=== AST (Before Semantic Analysis) ===\n";
    ASTPrinter::print(ast);
  }

  if (opts.stopAfterParse) {
    cleanupFile(preprocessedFile);
    return 0;
  }

  // Semantic Analysis
  SemanticAnalyzer semanticAnalyzer;
  semanticAnalyzer.analyze(ast);

  if (!semanticAnalyzer.success) {
    std::cerr << "velox: error: semantic analysis failed\n";
    for (const auto &error : semanticAnalyzer.errors) {
      std::cerr << "  " << error << "\n";
    }
    cleanupFile(preprocessedFile);
    return 1;
  }

  if (opts.verbose) {
    std::cout << "\n=== AST (After Semantic Analysis) ===\n";
    ASTPrinter::print(ast);
  }

  if (opts.stopAfterValidate) {
    cleanupFile(preprocessedFile);
    return 0;
  }

  // IR Generation
  Valor valor(semanticAnalyzer.label_counter);
  auto irProgram = valor.convertToIR(ast);

  if (opts.verbose) {
    std::cout << "\n=== IR ===\n";
    std::cout << irProgram->toString() << std::endl;
  }

  if (opts.stopAfterIR) {
    cleanupFile(preprocessedFile);
    return 0;
  }

  // Code Generation
  Codegen codegenerator;
  auto asmProgram = codegenerator.generateCode(irProgram);

  if (opts.stopAfterCodegen) {
    cleanupFile(preprocessedFile);
    return 0;
  }

  // Write Assembly File
  std::ofstream asmOutputFile(asmFile);
  if (!asmOutputFile.is_open()) {
    std::cerr << "velox: error: cannot create assembly file: " << asmFile
              << "\n";
    cleanupFile(preprocessedFile);
    return 1;
  }
  asmOutputFile << asmProgram->toString();
  asmOutputFile.close();

  cleanupFile(preprocessedFile);

  if (opts.stopAfterAsm) {
    if (!opts.outputFile.empty()) {
      std::rename(asmFile.c_str(), opts.outputFile.c_str());
    }
    return 0;
  }

  // Assemble to Object File
  std::string asmCmd = "gcc -m64 -c " + asmFile + " -o " + objFile + " 2>&1";
  ret = system(asmCmd.c_str());
  if (ret != 0) {
    std::cerr << "velox: error: assembly failed\n";
    cleanupFile(asmFile);
    return 1;
  }

  cleanupFile(asmFile);

  if (opts.stopAfterObject) {
    if (!opts.outputFile.empty()) {
      std::rename(objFile.c_str(), opts.outputFile.c_str());
    }
    return 0;
  }

  // Link to Executable
  std::string linkCmd = "gcc -m64 " + objFile + " -o " + exeFile;
  for (const auto &lib : opts.linkLibs) {
    linkCmd += " " + lib;
  }
  linkCmd += " 2>&1";

  ret = system(linkCmd.c_str());
  cleanupFile(objFile);

  if (ret != 0) {
    std::cerr << "velox: error: linking failed\n";
    return 1;
  }

  return 0;
}