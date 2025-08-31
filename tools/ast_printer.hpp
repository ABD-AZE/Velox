#pragma once
#include <iosfwd>

// Forward declare the root so user code can include this header
// without pulling in the entire AST.
struct AST_Node_program;

namespace velox_astpp {
  // Pretty-prints the AST to the provided output stream.
  void PrintAST(const AST_Node_program* root, std::ostream& os);
}
