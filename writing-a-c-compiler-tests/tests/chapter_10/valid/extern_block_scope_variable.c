// #ifdef SUPPRESS_WARNINGS
// #pragma GCC diagnostic ignored "-Wunused-variable"
// #endif
// defining foo after its usage won't work because we are doing semantic analysis in a single pass
int foo = 3;
int main(void) {
  int outer = 1;
  int foo = 0;
  if (outer) {
    /* You can declare a variable with linkage
     * multiple times in the same block;
     * these both refer to the 'foo' variable defined below
     */
    extern int foo;
    extern int foo;
    return foo;
  }
  return 0;
}
