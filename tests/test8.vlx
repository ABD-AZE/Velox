typedef struct Node { int v; struct Node* next; } Node;
int main(){
  int a = 7, b = 42U, c = 9l, d = 11Ul, e = 13LU, f = 0, g = 1;
  double x = .5, y = 7., z = 12.345, w = .0;
  char ch = '\n'; char q = '\''; char dq = '\"'; char bs = '\\';
  const char* s = "hello\tworld\n\"q\" \\\\ ok";
 a = a + 1; a += 2; ++a; a++; b = b - 1; b -= 3; --b; b--; c = c * 2; c *= 2; d = d / 2; d /= 2; e = e % 5; e %= 2;
  if ( (a == b) || (c != d) && (e < f) || (g > a) ) {
    a = (a << 1); b = (b >> 2); a <<= 1; b >>= 2;
    a = (a & b) | (c ^ d); a &= b; b |= c; c ^= d;
    if (!!a && ~b) a = a ? b : c;
  }
  printf("x=%f y=%f\n", x, y); 
  Node n; n.v = 10; n.next = &n;
  Node* p = &n; if (p->next->next == &n) { ... } 
  w = .123; z = 7.; x = .9;
  a = b / c;        /=
  return 0;
}

