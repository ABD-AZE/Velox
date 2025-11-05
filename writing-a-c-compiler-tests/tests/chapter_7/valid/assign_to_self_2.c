int main(void) {
  int a = 3;
  { int a = a = 3; }
  return a;
}